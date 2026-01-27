// Copyright 2024, Korstian Project. All Rights Reserved.

#include "Movement/MovementLogicComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Attributes/AttributeComponent.h"
#include "State/CharacterStateComponent.h"
#include "TimerManager.h"

UMovementLogicComponent::UMovementLogicComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SprintStaminaCost = 10.f;
	DodgeStaminaCost = 20.f;

	BackstepMontage = nullptr;
	RollMontage = nullptr;
}


void UMovementLogicComponent::BeginPlay()
{
	Super::BeginPlay();

	// Find and cache all necessary components from the owner
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		CMC = OwnerCharacter->GetCharacterMovement();
	}
	AttributeComp = GetOwner()->FindComponentByClass<UAttributeComponent>();
	StateComp = GetOwner()->FindComponentByClass<UCharacterStateComponent>();

	if (!OwnerCharacter || !CMC || !AttributeComp || !StateComp)
	{
		UE_LOG(LogTemp, Error, TEXT("MovementLogicComponent: Missing required component(s) on owner!"));
		SetComponentTickEnabled(false);
		return;
	}

	// Subscribe to the attribute change delegate
	AttributeComp->OnAttributeChanged.AddDynamic(this, &UMovementLogicComponent::OnStatUpdated);

	// Apply initial values
	ApplyMovementRates();
}

void UMovementLogicComponent::StartSprint()
{
	if (StateComp && StateComp->CanSprint())
	{
		// Check for sufficient stamina to start sprinting
		if (AttributeComp && AttributeComp->GetAttributeValue(EAttributeType::Stamina) > 0)
		{
			StateComp->SetState(ECharacterState::Sprinting);
			ApplyMovementRates();

			// Start stamina drain timer
			GetWorld()->GetTimerManager().SetTimer(SprintStaminaDrainTimer, this, &UMovementLogicComponent::DrainSprintStamina, 1.f, true);
		}
	}
}

void UMovementLogicComponent::StopSprint()
{
	if (StateComp && StateComp->IsSprinting())
	{
		StateComp->SetState(ECharacterState::Idle);
		ApplyMovementRates();

		// Stop stamina drain timer
		GetWorld()->GetTimerManager().ClearTimer(SprintStaminaDrainTimer);
	}
}

void UMovementLogicComponent::Dodge()
{
	if (StateComp && StateComp->CanDodge())
	{
		// Check for sufficient stamina
		if (AttributeComp && AttributeComp->GetAttributeValue(EAttributeType::Stamina) >= DodgeStaminaCost)
		{
			// Deduct stamina immediately
			FStatModifier StaminaMod(-DodgeStaminaCost, EModifierType::Add, this);
			AttributeComp->AddModifier(EAttributeType::Stamina, StaminaMod);

			// TODO: This should be a temporary modifier that is removed after a very short time
			// For now, it's a permanent reduction until a proper temporary modifier system is implemented.
			
			StateComp->SetState(ECharacterState::Dodging);
			
			UAnimMontage* MontageToPlay = nullptr;
			if (CMC->Velocity.SizeSquared() > 1.f)
			{
				MontageToPlay = RollMontage;
			}
			else
			{
				MontageToPlay = BackstepMontage;
			}

			if (MontageToPlay && OwnerCharacter)
			{
				OwnerCharacter->PlayAnimMontage(MontageToPlay);
				// A timer or anim notify should be used to set the state back to Idle
				// For now, let's use a simple timer.
				FTimerHandle DodgeEndTimer;
				float MontageLength = MontageToPlay->GetPlayLength();
				GetWorld()->GetTimerManager().SetTimer(DodgeEndTimer, [this]()
				{
					if (StateComp && StateComp->IsDodging())
					{
						StateComp->SetState(ECharacterState::Idle);
					}
				}, MontageLength, false);
			}
			else
			{
				// If no montage, just reset state
				StateComp->SetState(ECharacterState::Idle);
			}
		}
	}
}

void UMovementLogicComponent::ToggleCrouch()
{
	if (!OwnerCharacter || !StateComp) return;

	if (StateComp->GetStance() == ECharacterStance::Crouching)
	{
		OwnerCharacter->UnCrouch();
		StateComp->SetStance(ECharacterStance::Standing);
	}
	else
	{
		OwnerCharacter->Crouch();
		StateComp->SetStance(ECharacterStance::Crouching);
	}
	ApplyMovementRates();
}


void UMovementLogicComponent::OnStatUpdated(EAttributeType Type, float NewValue, float Delta)
{
	// Check if the updated stat is relevant to movement
	switch(Type)
	{
		case EAttributeType::WalkSpeedRate:
		case EAttributeType::SprintSpeedRate:
		case EAttributeType::JumpForceRate:
			ApplyMovementRates();
			break;
		case EAttributeType::Stamina:
			if (NewValue <= 0 && StateComp && StateComp->IsSprinting())
			{
				StopSprint(); // Stop sprinting if out of stamina
			}
			break;
		default:
			break;
	}
}

void UMovementLogicComponent::ApplyMovementRates()
{
	if (!CMC || !AttributeComp || !StateComp) return;
	
	float WalkSpeedRate = AttributeComp->GetAttributeValue(EAttributeType::WalkSpeedRate);
	float SprintSpeedRate = AttributeComp->GetAttributeValue(EAttributeType::SprintSpeedRate);
	float JumpForceRate = AttributeComp->GetAttributeValue(EAttributeType::JumpForceRate);
	
	if(StateComp->GetStance() == ECharacterStance::Crouching)
	{
		// Use CMC's built-in crouch speed
		CMC->MaxWalkSpeed = CMC->MaxWalkSpeedCrouched;
	}
	else if (StateComp->IsSprinting())
	{
		CMC->MaxWalkSpeed = BaseSprintSpeed * SprintSpeedRate;
	}
	else
	{
		CMC->MaxWalkSpeed = BaseWalkSpeed * WalkSpeedRate;
	}

	CMC->JumpZVelocity = BaseJumpZVelocity * JumpForceRate;
}


void UMovementLogicComponent::DrainSprintStamina()
{
	if (AttributeComp)
	{
		if (AttributeComp->GetAttributeValue(EAttributeType::Stamina) > 0)
		{
			FStatModifier StaminaDrain(-SprintStaminaCost, EModifierType::Add, this);
			AttributeComp->AddModifier(EAttributeType::Stamina, StaminaDrain);
			// This should also be a temporary modifier
		}
		else
		{
			StopSprint();
		}
	}
}
