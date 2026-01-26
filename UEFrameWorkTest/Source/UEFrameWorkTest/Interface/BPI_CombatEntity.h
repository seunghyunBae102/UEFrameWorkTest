// Ghost in the Monitor - Inner World Combat System
// BPI_CombatEntity.h
// 모든 전투 액터가 구현해야 하는 전투 인터페이스

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BPI_CombatEntity.generated.h"

// Forward Declarations
class AActor;
class UDA_InnerItem;

// Combat state enumeration
UENUM(BlueprintType)
enum class EBSCombatState : uint8
{
	Idle = 0        UMETA(DisplayName = "Idle"),
	Attacking = 1   UMETA(DisplayName = "Attacking"),
	Defending = 2   UMETA(DisplayName = "Defending"),
	Staggered = 3   UMETA(DisplayName = "Staggered"),
	Dead = 4        UMETA(DisplayName = "Dead")
};

UINTERFACE(MinimalAPI, Blueprintable)
class UBP_CombatEntity : public UInterface
{
	GENERATED_BODY()
};

/**
 * BPI_CombatEntity
 * 
 * Inner World의 모든 전투 액터가 구현해야 하는 전투 인터페이스.
 * Knight, Enemy, Boss 등이 이 인터페이스를 구현.
 * 
 * 역할:
 * - 전투 상태 관리 (Idle, Attacking, Defending, Staggered, Dead)
 * - 강인도(Poise) 시스템 관리
 * - 패링(Clash) 판정 로직
 * - 데미지 처리
 * - 사망 처리
 */
class UEFRAMEWORKTEST_API IBP_CombatEntity
{
	GENERATED_BODY()

public:
	/**
	 * GetCurrentPoise
	 * 현재 강인도 조회
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CombatEntity")
	float GetCurrentPoise() const;

	/**
	 * GetMaxPoise
	 * 최대 강인도 조회
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CombatEntity")
	float GetMaxPoise() const;

	/**
	 * ReceivePoiseDamage
	 * 강인도 데미지 수취
	 * 
	 * @param PoiseDamage - 강인도 데미지량
	 * @param Instigator - 공격자
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CombatEntity")
	void ReceivePoiseDamage(float PoiseDamage, AActor* Instigator);

	/**
	 * AttemptAttack
	 * 공격 시도
	 * 
	 * @param bIsHeavy - 무거운 공격 여부 (true: 무거운 공격, false: 경공격)
	 * @return 공격이 시작되었는지 여부
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CombatEntity")
	bool AttemptAttack(bool bIsHeavy);

	/**
	 * CheckClash
	 * 패링(Clash) 판정
	 * 무기 콜리전 오버랩 이벤트에서 호출됨.
	 * 
	 * @param OtherWeapon - 충돌한 무기 소유자 액터
	 * @return 패링이 성공했는지 여부
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CombatEntity")
	bool CheckClash(AActor* OtherWeapon);

	/**
	 * ToggleGuard
	 * 가드 상태 토글
	 * 
	 * @param bEnable - 가드 활성화 여부
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CombatEntity")
	void ToggleGuard(bool bEnable);

	/**
	 * IsGuardActive
	 * 가드 활성화 상태 조회
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CombatEntity")
	bool IsGuardActive() const;

	/**
	 * GetCurrentCombatState
	 * 현재 전투 상태 조회
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CombatEntity")
	EBSCombatState GetCurrentCombatState() const;

	/**
	 * OnStaggered
	 * 스태거(강인도 파괴) 이벤트
	 * 패링 실패 또는 강인도 0 도달 시 호출됨.
	 * 
	 * @param ImpactDirection - 임팩트 방향
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CombatEntity")
	void OnStaggered(FVector ImpactDirection);

	/**
	 * OnDeath
	 * 사망 이벤트
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CombatEntity")
	void OnDeath();

	/**
	 * IsAlive
	 * 생존 여부 조회
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CombatEntity")
	bool IsAlive() const;
};
