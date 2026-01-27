// Copyright 2024, Korstian Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

// Forward declarations to reduce header dependencies
class UAttributeComponent;
class UCharacterStateComponent;
class UEquipmentComponent;
class UAnimInstance;
class ABaseWeapon;
enum class EAttributeType : uint8;

/**
 * @class UCombatComponent
 * @brief 캐릭터의 모든 전투 행동(공격, 방어, 패리 등)을 총괄하는 컴포넌트입니다.
 * 입력에 따라 다른 컴포넌트(스탯, 상태, 장비)와 상호작용하여 실제 전투 로직을 실행하고,
 * 애니메이션 노티파이와 연동하여 정교한 공격 판정을 수행합니다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEFRAMEWORKTEST_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	// --- CACHED COMPONENTS (캐시된 컴포넌트) ---
	// BeginPlay 시점에 다른 컴포넌트들에 대한 포인터를 저장하여, 매번 찾는 비용을 절약합니다.
	UPROPERTY() TObjectPtr<UAttributeComponent> AttributeComp;
	UPROPERTY() TObjectPtr<UCharacterStateComponent> StateComp;
	UPROPERTY() TObjectPtr<UEquipmentComponent> EquipmentComp;
	UPROPERTY() TObjectPtr<UAnimInstance> AnimInstance;
	UPROPERTY() TObjectPtr<class ASoulCharacter> OwnerSoulCharacter;
	
public:
	// --- ATTACKS (공격) ---
	/** @brief 약공격을 시도합니다. 스태미나와 캐릭터 상태를 검증한 후, 콤보 인덱스에 맞는 공격을 실행합니다. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void LightAttack();
	
	/** @brief 강공격을 시도합니다. 약공격과 유사하게 스태미나와 상태를 검증 후 실행합니다. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void HeavyAttack();

	// --- DEFENSE (방어) ---
	/** @brief 가드를 시작합니다. 캐릭터 상태를 'Guarding'으로 변경하고, 관련 스탯(이동속도 등)을 조정하도록 요청합니다. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartGuard();

	/** @brief 가드를 중지합니다. 캐릭터 상태를 원래대로 되돌립니다. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StopGuard();

	/** @brief 패리를 시도합니다. 짧은 시간 동안 캐릭터를 'Parrying' 상태로 만듭니다. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Parry();

	// --- NOTIFY TRIGGERS (애니메이션 노티파이 연동) ---
	/** 
	 * @brief 애니메이션 노티파이에서 호출되어 공격 판정(Hit Detection)을 활성화합니다.
	 * 이 함수가 호출된 시점부터 EndAttack이 호출되기 전까지 무기 트레이스가 매 틱 수행됩니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Notify")
	void BeginAttack();
	
	/**
	 * @brief 애니메이션 노티파이에서 호출되어 공격 판정을 비활성화하고, 피격된 액터 목록을 초기화합니다. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Notify")
	void EndAttack();

	/** @brief 패리 애니메이션 중 패리 판정이 시작되는 시점에 애니메이션 노티파이에서 호출됩니다. */
	UFUNCTION(BlueprintCallable, Category = "Combat|Notify")
	void BeginParryWindow();

	/** @brief 패리 애니메이션 중 패리 판정이 끝나는 시점에 애니메이션 노티파이에서 호출됩니다. */
	UFUNCTION(BlueprintCallable, Category = "Combat|Notify")
	void EndParryWindow();
	
protected:
	// --- HIT DETECTION (공격 판정) ---
	/** @brief 현재 공격 판정이 활성화되었는지 여부를 나타냅니다. TickComponent에서 이 값이 true일 때만 PerformWeaponTrace를 호출합니다. */
	bool bIsHitboxActive = false;
	
	/** @brief 한 번의 공격(스윙) 동안 이미 피격된 액터들의 목록입니다. 동일한 적이 한 스윙에 여러 번 맞는 것을 방지합니다. */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> HitActors;

	/**
	 * @brief 현재 실행 중인 공격의 데이터(대미지 배율, 경직도 등)를 캐시합니다.
	 * LightAttack/HeavyAttack 함수에서 설정되며, PerformWeaponTrace에서 실제 대미지를 계산할 때 사용됩니다.
	 */
	const struct FAttackData* CurrentAttackData;

	/** @brief 무기의 소켓 위치를 기반으로 매 틱 트레이스를 수행하여 적과의 충돌을 감지합니다. */
	void PerformWeaponTrace();

protected:
	// --- STATS (스탯 연동) ---
	/** @brief AttributeComponent의 델리게이트에 바인딩되어, 스탯 변경 시 호출됩니다. 필요 시 전투 관련 캐시 데이터를 업데이트하는 데 사용될 수 있습니다. */
	UFUNCTION()
	void OnAttributeChanged(EAttributeType Type, float NewValue, float Delta);

private:
	// --- COMBO (콤보 시스템) ---
	/** @brief 현재 약공격 콤보의 단계를 추적합니다. (0 -> 1 -> 2 -> ...) */
	int32 LightAttackComboIndex = 0;
	/** @brief 현재 강공격 콤보의 단계를 추적합니다. */
	int32 HeavyAttackComboIndex = 0;
	
	/** @brief 콤보 유효 시간이 지나면 콤보 인덱스를 0으로 초기화합니다. */
	void ResetCombo();

	/** @brief ResetCombo 함수를 지연 호출하기 위한 타이머 핸들입니다. */
	FTimerHandle ComboResetTimer;
};
