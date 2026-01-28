// Copyright 2024, Korstian Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SoulCharacter.generated.h"

// Forward declarations
class UAttributeComponent;
class UCharacterStateComponent;
class UMovementLogicComponent;
class UParkourComponent;
class UMotionWarpingComponent;
class UEquipmentComponent;
class UCombatComponent;
class USphereComponent;
struct FInputActionValue;

/**
 * @class ASoulCharacter
 * @brief 이 프로젝트의 플레이어블 캐릭터 클래스입니다.
 * 모든 커스텀 게임플레이 컴포넌트(스탯, 상태, 무브먼트, 전투 등)를 소유하고 초기화하며,
 * 플레이어의 입력을 받아 각 컴포넌트의 기능에 연결하는 '중앙 허브' 역할을 합니다.
 * 즉, 개별 기능(장기)을 담당하는 컴포넌트들을 담는 하나의 '몸체'입니다.
 */
UCLASS()
class UEFRAMEWORKTEST_API ASoulCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASoulCharacter();

protected:
	// --- CORE COMPONENTS (핵심 컴포넌트) ---
	/** @brief 스탯 시스템: 캐릭터의 모든 수치(체력, 공격력 등)를 관리합니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAttributeComponent> AttributeComp;

	/** @brief 상태 시스템: 캐릭터의 현재 상태(공격중, 회피중 등)를 관리하고 행동 규칙을 강제합니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCharacterStateComponent> StateComp;
	
	// --- MOVEMENT COMPONENTS (이동 관련 컴포넌트) ---
	/** @brief 기본 이동 시스템: 달리기, 회피, 앉기 등 일반적인 이동 로직을 담당합니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMovementLogicComponent> MovementLogicComp;

	/** @brief 파쿠르 시스템: 벽 타기, 장애물 넘기 등 환경과 상호작용하는 특수 이동을 담당합니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UParkourComponent> ParkourComp;
	
	/** @brief 모션 워핑 시스템: 파쿠르와 같은 행동 시 애니메이션을 지형에 동적으로 맞춰주는 역할을 합니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComp;

	// --- COMBAT COMPONENTS (전투 관련 컴포넌트) ---
	/** @brief 장비 시스템: 무기 장착 및 해제 등 장비를 관리합니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEquipmentComponent> EquipmentComp;

	/** @brief 전투 시스템: 공격, 방어, 패리 등 실제 전투 로직을 수행합니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCombatComponent> CombatComp;

	/** @brief 패리 판정을 위한 충돌체입니다. 패리 애니메이션의 특정 구간에서만 활성화됩니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> ParryHitbox;

protected:
	virtual void BeginPlay() override;

	/** @brief 캐릭터가 대미지를 받을 때 호출되는 엔진 기본 함수를 오버라이드합니다. 받은 피해를 AttributeComponent에 전달하여 체력을 감소시키는 등의 처리를 합니다. */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	/** @brief 캐릭터가 공중에서 착지할 때 호출됩니다. 낙하 피해 계산 등을 위해 오버라이드합니다. */
	virtual void Landed(const FHitResult& Hit) override;

	/** @brief AttributeComponent로부터 강인도가 모두 소진되었다는 알림을 받았을 때 호출됩니다. */
	UFUNCTION()
	void OnPoiseDepleted();

	/** @brief 스턴(경직) 상태가 되었을 때 재생할 피격 반응 몽타주입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactionMontage;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** @brief 패리 충돌 박스의 활성화 여부를 설정합니다. CombatComponent의 애니메이션 노티파이에서 호출됩니다. */
	void SetParryHitboxEnabled(bool bIsEnabled);

	/** @brief 외부 요인(예: 패리 성공)에 의해 이 캐릭터를 강제로 스턴 상태로 만듭니다. */
	void GetStunned();

	// --- INLINE GETTERS (인라인 Getter 함수) ---
	// 다른 클래스에서 이 캐릭터의 컴포넌트들에 편리하고 효율적으로 접근할 수 있도록 제공하는 함수들입니다.
	FORCEINLINE UAttributeComponent* GetAttributeComponent() const { return AttributeComp; }
	FORCEINLINE UCharacterStateComponent* GetStateComponent() const { return StateComp; }
	FORCEINLINE UMovementLogicComponent* GetMovementLogicComponent() const { return MovementLogicComp; }
	FORCEINLINE UParkourComponent* GetParkourComponent() const { return ParkourComp; }
	FORCEINLINE UEquipmentComponent* GetEquipmentComponent() const { return EquipmentComp; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComp; }
};
