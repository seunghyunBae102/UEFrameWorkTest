// Copyright 2024, Korstian Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Attributes/AttributeSystemTypes.h"
#include "MovementLogicComponent.generated.h"

// Forward declarations
class ACharacter;
class UCharacterMovementComponent;
class UAttributeComponent;
class UCharacterStateComponent;

/**
 * @class UMovementLogicComponent
 * @brief 캐릭터의 기본 이동 로직(달리기, 회피, 앉기 등)을 관리하는 컴포넌트입니다.
 * 플레이어의 입력과 스탯 시스템(AttributeComponent)의 데이터를 기반으로,
 * 언리얼 엔진의 기본 CharacterMovementComponent(CMC)를 제어하는 '컨트롤러' 역할을 합니다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEFRAMEWORKTEST_API UMovementLogicComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMovementLogicComponent();

protected:
	virtual void BeginPlay() override;

private:
	// --- Cached component references (캐시된 컴포넌트 참조) ---
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CMC;
	UPROPERTY()
	TObjectPtr<UAttributeComponent> AttributeComp;
	UPROPERTY()
	TObjectPtr<UCharacterStateComponent> StateComp;
	
public:
	// --- Input-bound functions (입력 처리 함수) ---
	/** @brief 달리기를 시작합니다. 캐릭터 상태를 'Sprinting'으로 변경하고 지속적인 스태미나 소모를 시작합니다. */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	/** @brief 달리기를 멈춥니다. 캐릭터 상태를 원래대로 되돌리고 스태미나 소모를 중단합니다. */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprint();

	/** 
	 * @brief 회피(구르기 또는 백스텝)를 시도합니다.
	 * 스태미나와 캐릭터 상태를 검증한 후, 이동 입력 여부에 따라 적절한 루트모션 몽타주를 재생합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Dodge();

	/** @brief 앉기 상태를 토글합니다. CMC의 앉기 기능을 직접 호출합니다. */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ToggleCrouch();

protected:
	/**
	 * @brief AttributeComponent의 OnAttributeChanged 델리게이트에 바인딩되는 핸들러입니다.
	 * 이동 관련 스탯(예: WalkSpeedRate)이 변경될 때마다 호출되어, CMC의 값을 실시간으로 업데이트하는 '데이터 주입'의 핵심입니다.
	 */
	UFUNCTION()
	void OnStatUpdated(EAttributeType Type, float NewValue, float Delta);

	/** @brief 현재 AttributeComponent에 저장된 모든 이동 관련 배율(Rate)을 CMC에 적용합니다. BeginPlay 시와 스탯 업데이트 시 호출됩니다. */
	void ApplyMovementRates();
	
private:
	// --- Sprinting (달리기 관련) ---
	/** @brief 달리기 시 초당 소모될 스태미나의 양입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sprint")
	float SprintStaminaCost;

	/** @brief 달리기 중 스태미나를 주기적으로 소모시키기 위한 타이머 핸들입니다. */
	FTimerHandle SprintStaminaDrainTimer;

	/** @brief 달리기 중 스태미나를 실제로 감소시키는 함수입니다. 타이머에 의해 주기적으로 호출됩니다. */
	void DrainSprintStamina();
	
	// --- Dodging (회피 관련) ---
	/** @brief 회피 행동 시 한 번에 소모될 스태미나의 양입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dodge")
	float DodgeStaminaCost;

	/** @brief 제자리 회피(백스텝) 시 재생할 애니메이션 몽타주입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dodge")
	TObjectPtr<UAnimMontage> BackstepMontage;
	
	/** @brief 이동 중 회피(구르기) 시 재생할 애니메이션 몽타주입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dodge")
	TObjectPtr<UAnimMontage> RollMontage;

	// --- Base movement values (기본 이동 수치) ---
	// 이 값들은 스탯 시스템에서 받은 '배율(Rate)'과 곱해져 최종 이동 속도/능력을 결정합니다.
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Defaults")
	float BaseWalkSpeed = 600.f;
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Defaults")
	float BaseSprintSpeed = 900.f;
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Defaults")
	float BaseJumpZVelocity = 600.f;
};
