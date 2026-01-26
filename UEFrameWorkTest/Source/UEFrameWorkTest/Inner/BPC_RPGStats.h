// GHOST IN THE MONITOR - Inner World RPG Stats Component
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BPC_RPGStats.generated.h"

// Forward Declarations
class UDA_CharacterBaseStats;

// Delegate for poise change events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoiseChanged, float, NewPoise, float, MaxPoise);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPoiseBroken);

/**
 * BPC_RPGStats
 *
 * Inner World 캐릭터의 모든 RPG 관련 스탯(체력, 강인도 등)을 관리하는 핵심 로직 컴포넌트입니다.
 * 이 프로토타입에서는 '강인도(Poise)' 시스템만 우선 구현합니다.
 *
 * 역할:
 * - DA_CharacterBaseStats 에셋에서 기본 수치를 가져와 초기화합니다.
 * - 강인도 피해를 적용하고 현재 강인도를 관리합니다.
 * - Tick을 사용하지 않고 Timer를 이용해 강인도 회복 로직을 처리합니다.
 * - 강인도 변화 및 파괴 이벤트를 외부에 알립니다. (Event-Driven)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UEFRAMEWORKTEST_API UBPC_RPGStats : public UActorComponent
{
	GENERATED_BODY()

public:
	UBPC_RPGStats();

	/**
	 * @brief 이 캐릭터의 기본 스탯을 정의하는 데이터 에셋입니다.
	 * 블루프린트에서 이 컴포넌트를 추가할 때 반드시 할당해야 합니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UDA_CharacterBaseStats> BaseStatsDataAsset;

	/** @brief 강인도 수치가 변경될 때 호출되는 이벤트입니다. (UI 업데이트용) */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPoiseChanged OnPoiseChanged;

	/** @brief 강인도가 0이 되어 파괴되었을 때 호출되는 이벤트입니다. (스태거 애니메이션 재생용) */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPoiseBroken OnPoiseBroken;

	/**
	 * @brief 캐릭터에게 강인도 피해를 적용합니다.
	 * @param DamageAmount 적용할 강인도 피해량
	 */
	UFUNCTION(BlueprintCallable, Category = "RPGStats|Poise")
	void ApplyPoiseDamage(float DamageAmount);

	UFUNCTION(BlueprintPure, Category = "RPGStats|Poise")
	float GetCurrentPoise() const { return CurrentPoise; }

	UFUNCTION(BlueprintPure, Category = "RPGStats|Poise")
	float GetMaxPoise() const { return MaxPoise; }

protected:
	virtual void BeginPlay() override;

private:
	/** @brief 현재 강인도 수치입니다. */
	UPROPERTY(VisibleAnywhere, Category = "RPGStats|Poise", meta = (AllowPrivateAccess = "true"))
	float CurrentPoise;

	/** @brief 최대 강인도 수치입니다. BeginPlay 시 데이터 에셋에서 초기화됩니다. */
	UPROPERTY(VisibleAnywhere, Category = "RPGStats|Poise", meta = (AllowPrivateAccess = "true"))
	float MaxPoise;

	/** @brief 강인도 회복 딜레이를 처리하기 위한 타이머 핸들입니다. */
	FTimerHandle PoiseRecoveryDelayTimerHandle;

	/** @brief 강인도를 지속적으로 회복시키기 위한 타이머 핸들입니다. */
	FTimerHandle PoiseRecoveryTickTimerHandle;

	/**
	 * @brief 강인도 회복 딜레이가 끝난 후 호출되어, 실제 회복을 시작합니다.
	 */
	void StartPoiseRecovery();

	/**
	 * @brief PoiseRecoveryTickTimerHandle에 의해 주기적으로 호출되어 강인도를 회복시킵니다.
	 */
	void RecoverPoiseTick();
};