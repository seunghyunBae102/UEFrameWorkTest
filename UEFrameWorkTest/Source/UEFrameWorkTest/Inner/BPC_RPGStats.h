// GHOST IN THE MONITOR - Inner World RPG Stats Component
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/DA_InnerEquipment.h"
#include "GameplayTagContainer.h"
#include "BPC_RPGStats.generated.h"

/**
 * BPC_RPGStats - RPG 수치 계산 엔진
 * 
 * 역할:
 * - 4대 스탯(힘, 민, 지, 체) 관리
 * - 장비 보정치 계산
 * - 최종 데미지 연산
 * - 강인도(Poise) 시스템 관리
 * 
 * 규칙:
 * - 모든 스탯은 데이터 에셋(UDA_InnerEquipment)에서 받아옴
 * - bool 변수 대신 GameplayTags 사용 (예: "State.Dead", "State.Stagger")
 * - Tick 비활성화 (이벤트/함수 호출로만 작동)
 * - 타이머로 강인도 회복 관리
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UEFRAMEWORKTEST_API UBP_RPGStats : public UActorComponent
{
	GENERATED_BODY()

public:
	UBP_RPGStats();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ==================== Properties ====================

	// 기본 스탯 (힘, 민, 지, 체)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FInnerAttributes BaseAttributes;

	// 현재 강인도 (Poise)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentPoise = 100.0f;

	// 최대 강인도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxPoise = 100.0f;

	// 강인도 회복 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float PoiseRecoveryDelay = 5.0f;

	// 초당 강인도 회복량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float PoiseRecoveryRate = 20.0f;

	// 현재 장착 중인 장비들
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Equipment")
	TArray<UDA_InnerEquipment*> EquippedGear;

	// 상태 태그 (State.Dead, State.Stagger, State.Poisoned 등)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	FGameplayTagContainer StateTagContainer;

	// ==================== Events ====================

	// 스탯이 변경되었을 때 (예: 장비 착용)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatsChanged, FInnerAttributes, NewAttributes, float, NewMaxPoise);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStatsChanged OnStatsChanged;

	// 강인도가 변경되었을 때
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoiseChanged, float, CurrentPoise, float, MaxPoise);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPoiseChanged OnPoiseChanged;

	// 강인도가 0이 되어 Stagger 상태로 진입했을 때
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaggerState);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStaggerState OnStaggerState;

	// ==================== Methods ====================

	/**
	 * 장비 착용 (스탯 보정 적용)
	 * @param Gear: 장착할 장비
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipGear(UDA_InnerEquipment* Gear);

	/**
	 * 장비 해제 (스탯 보정 제거)
	 * @param Gear: 해제할 장비
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UnequipGear(UDA_InnerEquipment* Gear);

	/**
	 * 최종 공격력 계산 (보정치 적용)
	 * @param Weapon: 무기 데이터 에셋
	 * @return 최종 물리 공격력
	 */
	UFUNCTION(BlueprintPure, Category = "Calculation")
	float CalculateAttackPower(UDA_InnerEquipment* Weapon);

	/**
	 * 최종 마법 공격력 계산 (보정치 적용)
	 * @param Weapon: 무기 데이터 에셋
	 * @return 최종 마법 공격력
	 */
	UFUNCTION(BlueprintPure, Category = "Calculation")
	float CalculateMagicPower(UDA_InnerEquipment* Weapon);

	/**
	 * 강인도 피해 적용
	 * @param PoiseDamage: 입을 강인도 피해
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void TakePoiseDamage(float PoiseDamage);

	/**
	 * 강인도 회복
	 * @param Amount: 회복할 강인도
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecoverPoise(float Amount);

	/**
	 * Stagger 상태에서 벗어나기
	 */
	UFUNCTION(BlueprintCallable, Category = "State")
	void ExitStagger();

	/**
	 * 특정 상태 태그 추가
	 */
	UFUNCTION(BlueprintCallable, Category = "State")
	void AddStateTag(const FGameplayTag& Tag);

	/**
	 * 특정 상태 태그 제거
	 */
	UFUNCTION(BlueprintCallable, Category = "State")
	void RemoveStateTag(const FGameplayTag& Tag);

	/**
	 * 특정 상태가 활성화되어 있는지 확인
	 */
	UFUNCTION(BlueprintPure, Category = "State")
	bool HasStateTag(const FGameplayTag& Tag) const;

	/**
	 * 현재 강인도 조회
	 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetCurrentPoise() const { return CurrentPoise; }

	/**
	 * 현재 기본 스탯 조회
	 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	FInnerAttributes GetBaseAttributes() const { return BaseAttributes; }

	/**
	 * Stagger 상태 확인 (편의 함수)
	 */
	UFUNCTION(BlueprintPure, Category = "State")
	bool IsStaggering() const;

private:
	// 강인도 회복 타이머 핸들
	FTimerHandle PoiseRecoveryTimerHandle;

	/**
	 * 강인도 회복 틱 함수
	 */
	void TickPoiseRecovery();

	/**
	 * 특정 스탯에 대한 최종 보정값 계산
	 * @param AttributeType: 계산할 스탯 종류
	 */
	float CalculateTotalScaling(EInnerAttributeType AttributeType);

	/**
	 * 스탯 변경 시 이벤트 브로드캐스트
	 */
	void BroadcastStatsChange();
};
