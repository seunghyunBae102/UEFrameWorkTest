// Copyright 2024, Korstian Project. All Rights Reserved.

#include "Combat/CombatComponent.h"
#include "Attributes/AttributeComponent.h"
#include "State/CharacterStateComponent.h"
#include "Combat/EquipmentComponent.h"
#include "Data/WeaponDataAsset.h"
#include "Combat/BaseWeapon.h"
#include "Player/SoulCharacter.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerSoulCharacter = Cast<ASoulCharacter>(GetOwner());
	if (OwnerSoulCharacter)
	{
		AttributeComp = OwnerSoulCharacter->GetAttributeComponent();
		StateComp = OwnerSoulCharacter->GetStateComponent();
		EquipmentComp = OwnerSoulCharacter->GetEquipmentComponent();
		if (OwnerSoulCharacter->GetMesh())
		{
			AnimInstance = OwnerSoulCharacter->GetMesh()->GetAnimInstance();
		}
	}

	if (!AttributeComp || !StateComp || !EquipmentComp || !AnimInstance || !OwnerSoulCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("CombatComponent: Missing required component(s) on owner! Disabling tick."));
		SetComponentTickEnabled(false);
		return;
	}

	if (AttributeComp)
	{
		AttributeComp->OnAttributeChanged.AddDynamic(this, &UCombatComponent::OnAttributeChanged);
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsHitboxActive)
	{
		PerformWeaponTrace();
	}
}

void UCombatComponent::LightAttack()
{
	if (!StateComp->CanAttack()) return;
	
	UWeaponDataAsset* WeaponData = EquipmentComp->GetCurrentWeaponData();
	if (!WeaponData || WeaponData->LightAttacks.Num() == 0) return;

	// Reset combo if the current combo is finished, allowing it to loop.
	if (LightAttackComboIndex >= WeaponData->LightAttacks.Num())
	{
		LightAttackComboIndex = 0;
	}

	const FAttackData& AttackData = WeaponData->LightAttacks[LightAttackComboIndex];
	
	if (AttributeComp->GetAttributeValue(EAttributeType::Stamina) < AttackData.StaminaCost) return;
	
	// --- All checks passed, perform the attack ---
	CurrentAttackData = &AttackData;
	
	// A non-reversible modifier from a null source is used for one-time costs like stamina.
	AttributeComp->AddModifier(EAttributeType::Stamina, FStatModifier(-AttackData.StaminaCost, EModifierType::Add, nullptr));
	StateComp->SetState(ECharacterState::Attacking);
	
	AnimInstance->Montage_Play(AttackData.Montage);

	LightAttackComboIndex++;
	HeavyAttackComboIndex = 0; // Reset other attack's combo
	
	// Set a timer to reset the combo window
	GetWorld()->GetTimerManager().SetTimer(ComboResetTimer, this, &UCombatComponent::ResetCombo, 1.2f, false);
}

void UCombatComponent::HeavyAttack()
{
	if (!StateComp->CanAttack()) return;
	
	UWeaponDataAsset* WeaponData = EquipmentComp->GetCurrentWeaponData();
	if (!WeaponData || WeaponData->HeavyAttacks.Num() == 0) return;

	// Reset combo if the current combo is finished, allowing it to loop.
	if (HeavyAttackComboIndex >= WeaponData->HeavyAttacks.Num())
	{
		HeavyAttackComboIndex = 0;
	}

	const FAttackData& AttackData = WeaponData->HeavyAttacks[HeavyAttackComboIndex];
	
	if (AttributeComp->GetAttributeValue(EAttributeType::Stamina) < AttackData.StaminaCost) return;
	
	// --- All checks passed, perform the attack ---
	CurrentAttackData = &AttackData;

	// A non-reversible modifier from a null source is used for one-time costs like stamina.
	AttributeComp->AddModifier(EAttributeType::Stamina, FStatModifier(-AttackData.StaminaCost, EModifierType::Add, nullptr));
	StateComp->SetState(ECharacterState::Attacking);
	
	AnimInstance->Montage_Play(AttackData.Montage);

	HeavyAttackComboIndex++;
	LightAttackComboIndex = 0; // Reset other attack's combo

	// Set a timer to reset the combo window
	GetWorld()->GetTimerManager().SetTimer(ComboResetTimer, this, &UCombatComponent::ResetCombo, 1.5f, false);
}

void UCombatComponent::StartGuard()
{
	if (StateComp->GetState() == ECharacterState::Idle)
	{
		StateComp->SetState(ECharacterState::Guarding);
	}
}

void UCombatComponent::StopGuard()
{
	if (StateComp->GetState() == ECharacterState::Guarding)
	{
		StateComp->SetState(ECharacterState::Idle);
	}
}

void UCombatComponent::Parry()
{
	if (!StateComp->CanAttack()) return; // Can't parry if stunned/attacking

	UWeaponDataAsset* WeaponData = EquipmentComp->GetCurrentWeaponData();
	if (!WeaponData || !WeaponData->ParryMontage) return;
	
	StateComp->SetState(ECharacterState::Parrying);
	AnimInstance->Montage_Play(WeaponData->ParryMontage);
}

void UCombatComponent::BeginAttack()
{
	bIsHitboxActive = true;
	HitActors.Empty();
}

void UCombatComponent::EndAttack()
{
	bIsHitboxActive = false;
}

void UCombatComponent::BeginParryWindow()
{
	if (OwnerSoulCharacter)
	{
		OwnerSoulCharacter->SetParryHitboxEnabled(true);
	}
}

void UCombatComponent::EndParryWindow()
{
	if (OwnerSoulCharacter)
	{
		OwnerSoulCharacter->SetParryHitboxEnabled(false);
	}
}

void UCombatComponent::PerformWeaponTrace()
{
	AActor* WeaponActor = EquipmentComp->GetCurrentWeaponActor();
	if (!WeaponActor || !CurrentAttackData) return;
	
	// We need a proper interface or cast here. For now, assuming it's ABaseWeapon.
	ABaseWeapon* BaseWeapon = Cast<ABaseWeapon>(WeaponActor);
	if (!BaseWeapon) return;

	FVector Start = BaseWeapon->GetTraceStartLocation();
	FVector End = BaseWeapon->GetTraceEndLocation();
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());

	TArray<FHitResult> Hits;
	UKismetSystemLibrary::SphereTraceMulti(this, Start, End, 20.f, UEngineTypes::ConvertToTraceType(ECC_Pawn), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, Hits, true);

	for (const FHitResult& Hit : Hits)
	{
		if (Hit.GetActor() && !HitActors.Contains(Hit.GetActor()))
		{
			HitActors.Add(Hit.GetActor());

			// Calculate Damage
			float BaseDamage = AttributeComp->GetAttributeValue(EAttributeType::AttackPower);
			float TotalDamage = BaseDamage * CurrentAttackData->DamageMultiplier;

			// Apply Damage
			UGameplayStatics::ApplyPointDamage(Hit.GetActor(), TotalDamage, GetOwner()->GetActorForwardVector(), Hit, GetOwner()->GetInstigatorController(), GetOwner(), nullptr);

			// Apply Poise Damage
			if (UAttributeComponent* TargetAttributes = Hit.GetActor()->FindComponentByClass<UAttributeComponent>())
			{
				TargetAttributes->AddModifier(EAttributeType::Poise, FStatModifier(-CurrentAttackData->PoiseDamage, EModifierType::Add, GetOwner()));
			}
		}
	}
}


void UCombatComponent::ResetCombo()
{
	LightAttackComboIndex = 0;
	HeavyAttackComboIndex = 0;
	GetWorld()->GetTimerManager().ClearTimer(ComboResetTimer);
}

void UCombatComponent::OnAttributeChanged(EAttributeType Type, float NewValue, float Delta)
{
	// This function is called whenever an attribute changes.
	// We can use this to cache values that are derived from stats, like final attack power.
	if (Type == EAttributeType::AttackPower)
	{
		// UE_LOG(LogTemp, Log, TEXT("Attack Power changed to %f"), NewValue);
		// CachedAttackPower = NewValue; // Update a cached variable
	}
}
