// Copyright 2024, Korstian Project. All Rights Reserved.

#include "Movement/ParkourComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Attributes/AttributeComponent.h"
#include "State/CharacterStateComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MotionWarpingComponent.h"

UParkourComponent::UParkourComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UParkourComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		CMC = OwnerCharacter->GetCharacterMovement();
		MotionWarpingComp = OwnerCharacter->FindComponentByClass<UMotionWarpingComponent>();
	}
	AttributeComp = GetOwner()->FindComponentByClass<UAttributeComponent>();
	StateComp = GetOwner()->FindComponentByClass<UCharacterStateComponent>();

	if (!OwnerCharacter || !CMC || !AttributeComp || !StateComp)
	{
		UE_LOG(LogTemp, Error, TEXT("ParkourComponent: Missing required component(s) on owner! Disabling."));
		SetComponentTickEnabled(false);
		return;
	}
	
	// Motion Warping is essential for this component. Add it if it doesn't exist.
	if(!MotionWarpingComp)
	{
		MotionWarpingComp = NewObject<UMotionWarpingComponent>(OwnerCharacter, TEXT("MotionWarpingComp"));
		MotionWarpingComp->RegisterComponent();
	}

	// Subscribe to stat changes
	AttributeComp->OnAttributeChanged.AddDynamic(this, &UParkourComponent::SyncParkourStats);
	SyncParkourStats(EAttributeType::Dexterity, AttributeComp->GetAttributeValue(EAttributeType::Dexterity), 0);
}

bool UParkourComponent::TryParkour()
{
	if (!StateComp || !StateComp->CanJump()) // Use CanJump as a general "can I do an action" check
	{
		return false;
	}

	// 1. Check for a Ledge/Mantle
	FLedgeDetectionResult LedgeResult = DetectLedge();
	if (LedgeResult.bLedgeFound)
	{
		PerformMantle(LedgeResult);
		return true;
	}

	// 2. Check for a Vault
	FHitResult VaultHit;
	if (DetectVaultableObject(VaultHit))
	{
		PerformVault(VaultHit);
		return true;
	}

	return false;
}

bool UParkourComponent::DetectVaultableObject(FHitResult& OutHit)
{
	if (!OwnerCharacter) return false;

	const FVector Start = OwnerCharacter->GetActorLocation();
	const FVector End = Start + OwnerCharacter->GetActorForwardVector() * VaultTraceDistance;
	const float Radius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnerCharacter);

	// First trace to find a wall/obstacle
	if (UKismetSystemLibrary::SphereTraceSingle(this, Start, End, Radius, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHit, true))
	{
		// Check if the obstacle is at a vaultable height
		float ObstacleHeight = OutHit.ImpactPoint.Z - OwnerCharacter->GetActorLocation().Z;
		if (ObstacleHeight >= MinVaultHeight && ObstacleHeight <= MaxVaultHeight)
		{
			return true;
		}
	}
	return false;
}


FLedgeDetectionResult UParkourComponent::DetectLedge()
{
	FLedgeDetectionResult Result;
	if (!OwnerCharacter) return Result;

	const FVector Start = OwnerCharacter->GetActorLocation() + FVector::UpVector * MaxLedgeHeight;
	const FVector End = Start + OwnerCharacter->GetActorForwardVector() * LedgeTraceDistance;
	const float Radius = 15.f;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnerCharacter);
	FHitResult ForwardHit;

	// 1. Forward trace to find a wall
	if (UKismetSystemLibrary::SphereTraceSingle(this, Start, End, Radius, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, ForwardHit, true))
	{
		const FVector DownTraceStart = ForwardHit.ImpactPoint + ForwardHit.ImpactNormal * -15.f + FVector::UpVector * 5.f;
		const FVector DownTraceEnd = DownTraceStart + FVector::DownVector * (MaxLedgeHeight - MinLedgeHeight + 5.f);
		FHitResult DownHit;

		// 2. Downward trace to find the ledge surface
		if (UKismetSystemLibrary::LineTraceSingle(this, DownTraceStart, DownTraceEnd, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, DownHit, true))
		{
			Result.bLedgeFound = true;
			Result.LedgeLocation = DownHit.ImpactPoint;
			Result.LedgeNormal = DownHit.ImpactNormal;
			return Result;
		}
	}

	return Result;
}


void UParkourComponent::PerformVault(const FHitResult& ObstacleHit)
{
	if (!StateComp || !AttributeComp || !MotionWarpingComp || !VaultMontage) return;

	StateComp->SetState(ECharacterState::Vaulting);
	CMC->SetMovementMode(MOVE_Flying);

	// Setup Motion Warping
	FMotionWarpingTarget WarpTarget;
	WarpTarget.Name = TEXT("VaultTarget");
	WarpTarget.Location = ObstacleHit.ImpactPoint + OwnerCharacter->GetActorForwardVector() * 80.f; // Land just past the obstacle
	WarpTarget.Rotation = OwnerCharacter->GetActorRotation();
	MotionWarpingComp->AddOrUpdateWarpTarget(WarpTarget);

	float PlayRate = AttributeComp->GetAttributeValue(EAttributeType::Dexterity) / 10.f; // Example calculation
	OwnerCharacter->PlayAnimMontage(VaultMontage, FMath::Max(0.8f, PlayRate));

	FTimerHandle FinishTimer;
	GetWorld()->GetTimerManager().SetTimer(FinishTimer, this, &UParkourComponent::FinishParkourAction, VaultMontage->GetPlayLength() / PlayRate, false);
}

void UParkourComponent::PerformMantle(const FLedgeDetectionResult& LedgeResult)
{
	if (!StateComp || !AttributeComp || !MotionWarpingComp || !MantleMontage) return;

	StateComp->SetState(ECharacterState::Hanging); // Intermediate state
	CMC->SetMovementMode(MOVE_Flying);

	// Setup Motion Warping
	FMotionWarpingTarget WarpTarget;
	WarpTarget.Name = TEXT("MantleTarget");
	WarpTarget.Location = LedgeResult.LedgeLocation;
	WarpTarget.Rotation = LedgeResult.LedgeNormal.Rotation();
	// Adjust for capsule size to align the character properly with the ledge
	WarpTarget.Location += WarpTarget.Rotation.Vector() * OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(); 
	
	MotionWarpingComp->AddOrUpdateWarpTarget(WarpTarget);
	
	float PlayRate = AttributeComp->GetAttributeValue(EAttributeType::Dexterity) / 10.f;
	OwnerCharacter->PlayAnimMontage(MantleMontage, FMath::Max(0.8f, PlayRate));
	
	FTimerHandle FinishTimer;
	GetWorld()->GetTimerManager().SetTimer(FinishTimer, this, &UParkourComponent::FinishParkourAction, MantleMontage->GetPlayLength() / PlayRate, false);
}


void UParkourComponent::SyncParkourStats(EAttributeType Type, float NewValue, float Delta)
{
	if (!AttributeComp) return;

	// This is where we would update internal variables based on stat changes
	// For example, if we had a variable for vault speed, we'd update it here
	// based on the new Dexterity value.
	// For now, the values are read directly in the Perform... actions.
}

void UParkourComponent::FinishParkourAction()
{
	if (StateComp)
	{
		StateComp->SetState(ECharacterState::Idle);
	}
	if (CMC)
	{
		CMC->SetMovementMode(MOVE_Walking);
	}
}
