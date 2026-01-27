// Copyright 2024, Korstian Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "State/CharacterStateTypes.h"
#include "CharacterStateComponent.generated.h"

/**
 * @brief 캐릭터의 상태가 변경될 때 브로드캐스트되는 델리게이트입니다.
 * @param NewState 새로 변경된 상태
 * @param PrevState 이전 상태
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateChanged, ECharacterState, NewState, ECharacterState, PrevState);

/**
 * @class UCharacterStateComponent
 * @brief 캐릭터의 '상태'를 관리하고, 상태에 기반한 '행동 규칙'을 강제하는 중앙 관제 컴포넌트입니다.
 * CombatComponent, MovementComponent 등 다른 컴포넌트들은 어떤 행동을 하기 전에 반드시 이 컴포넌트에게
 * 해당 행동이 현재 가능한지(`CanAttack()`, `CanMove()` 등)를 물어봐야 합니다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEFRAMEWORKTEST_API UCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterStateComponent();

	/** @brief 캐릭터의 상태(ECharacterState)가 변경될 때마다 호출됩니다. UI나 다른 게임플레이 로직이 상태 변화에 반응하는 데 사용됩니다. */
	UPROPERTY(BlueprintAssignable, Category = "State")
	FOnStateChanged OnStateChanged;

protected:
	/** @brief 캐릭터의 현재 게임플레이 상태입니다. 이 값은 멀티플레이어 환경을 위해 서버에서 클라이언트로 복제(Replicate)됩니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_State, Category = "State")
	ECharacterState CurrentState;
	
	/** @brief 캐릭터의 현재 자세입니다. 주로 애니메이션 제어에 사용되며, 이 값 또한 복제됩니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_Stance, Category = "State")
	ECharacterStance CurrentStance;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/** @brief 캐릭터의 상태를 새로운 상태로 변경합니다. 서버에서만 호출되어야 합니다. */
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetState(ECharacterState NewState);

	/** @brief 현재 캐릭터의 상태를 가져옵니다. */
	UFUNCTION(BlueprintPure, Category = "State")
	ECharacterState GetState() const { return CurrentState; }

	/** @brief 캐릭터의 자세를 새로운 자세로 변경합니다. */
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetStance(ECharacterStance NewStance);
	
	/** @brief 현재 캐릭터의 자세를 가져옵니다. */
	UFUNCTION(BlueprintPure, Category = "State")
	ECharacterStance GetStance() const { return CurrentStance; }

	// --- State query functions (상태 질의 함수) ---
	/** @brief 현재 달리는 중인지 확인하는 간단한 편의 함수입니다. */
	UFUNCTION(BlueprintPure, Category = "State|Queries")
	bool IsSprinting() const { return CurrentState == ECharacterState::Sprinting; }
	
	/** @brief 현재 회피 중인지 확인합니다. */
	UFUNCTION(BlueprintPure, Category = "State|Queries")
	bool IsDodging() const { return CurrentState == ECharacterState::Dodging; }

	/** @brief 현재 스턴(경직) 상태인지 확인합니다. */
	UFUNCTION(BlueprintPure, Category = "State|Queries")
	bool IsStunned() const { return CurrentState == ECharacterState::Stunned; }
	
	/** @brief 현재 죽은 상태인지 확인합니다. */
	UFUNCTION(BlueprintPure, Category = "State|Queries")
	bool IsDead() const { return CurrentState == ECharacterState::Dead; }

	// --- Action permission functions (행동 허가 함수) ---
	// 다른 컴포넌트들이 행동을 시작하기 전 '반드시' 호출해야 하는 함수들입니다.
	
	/** @brief 현재 이동이 가능한 상태인지 확인합니다. (예: 공격, 스턴, 죽은 상태가 아니어야 함) */
	UFUNCTION(BlueprintPure, Category = "State|Permissions")
	bool CanMove() const;

	/** @brief 현재 달리기를 시작할 수 있는 상태인지 확인합니다. */
	UFUNCTION(BlueprintPure, Category = "State|Permissions")
	bool CanSprint() const;
	
	/** @brief 현재 회피를 할 수 있는 상태인지 확인합니다. (예: 스턴, 회피, 공격 중이 아니어야 함) */
	UFUNCTION(BlueprintPure, Category = "State|Permissions")
	bool CanDodge() const;
	
	/** @brief 현재 공격을 할 수 있는 상태인지 확인합니다. */
	UFUNCTION(BlueprintPure, Category = "State|Permissions")
	bool CanAttack() const;
	
	/** @brief 현재 점프를 할 수 있는 상태인지 확인합니다. */
	UFUNCTION(BlueprintPure, Category = "State|Permissions")
	bool CanJump() const;


private:
	/** @brief 서버에서 CurrentState가 변경되어 클라이언트로 복제될 때 클라이언트 측에서 자동으로 호출되는 함수입니다. 상태 변경에 따른 시각적 효과 등을 처리합니다. */
	UFUNCTION()
	void OnRep_State(ECharacterState PrevState);
	
	/** @brief CurrentStance가 클라이언트로 복제될 때 호출되는 함수입니다. */
	UFUNCTION()
	void OnRep_Stance(ECharacterStance PrevStance);
};
