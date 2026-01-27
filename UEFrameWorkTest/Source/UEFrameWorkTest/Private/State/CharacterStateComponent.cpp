// Copyright 2024, Korstian Project. All Rights Reserved.

#include "State/CharacterStateComponent.h"
#include "Net/UnrealNetwork.h"

UCharacterStateComponent::UCharacterStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	CurrentState = ECharacterState::Idle;
	CurrentStance = ECharacterStance::Standing;
}

void UCharacterStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterStateComponent, CurrentState);
	DOREPLIFETIME(UCharacterStateComponent, CurrentStance);
}

void UCharacterStateComponent::SetState(ECharacterState NewState)
{
	if (GetOwner()->HasAuthority() && CurrentState != NewState)
	{
		ECharacterState PrevState = CurrentState;
		CurrentState = NewState;
		OnRep_State(PrevState);
	}
}

void UCharacterStateComponent::SetStance(ECharacterStance NewStance)
{
	if (GetOwner()->HasAuthority() && CurrentStance != NewStance)
	{
		ECharacterStance PrevStance = CurrentStance;
		CurrentStance = NewStance;
		OnRep_Stance(PrevStance);
	}
}

void UCharacterStateComponent::OnRep_State(ECharacterState PrevState)
{
	OnStateChanged.Broadcast(CurrentState, PrevState);
}

void UCharacterStateComponent::OnRep_Stance(ECharacterStance PrevStance)
{
	// Optional: Could add a separate OnStanceChanged delegate if needed
}


bool UCharacterStateComponent::CanMove() const
{
	// Can't move while dodging, stunned, or dead
	return !IsDodging() && !IsStunned() && !IsDead();
}

bool UCharacterStateComponent::CanSprint() const
{
	// Can only sprint when idle (not attacking, dodging, etc.) and standing
	return CurrentState == ECharacterState::Idle && CurrentStance == ECharacterStance::Standing;
}

bool UCharacterStateComponent::CanDodge() const
{
	// Can't dodge if already dodging, stunned, dead, or in the middle of an attack
	// This can be expanded based on game design
	return CurrentState == ECharacterState::Idle || CurrentState == ECharacterState::Sprinting || CurrentState == ECharacterState::Crouching;
}

bool UCharacterStateComponent::CanAttack() const
{
	// Allow attacking from idle or sprinting (for a sprint attack)
	return CurrentState == ECharacterState::Idle || CurrentState == ECharacterState::Sprinting;
}

bool UCharacterStateComponent::CanJump() const
{
	// Can't jump while dodging, stunned, or dead
	return !IsDodging() && !IsStunned() && !IsDead();
}
