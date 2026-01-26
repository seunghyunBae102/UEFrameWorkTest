// Ghost in the Monitor - Inner World Combat System
// BPC_InnerCombat.h
// 전투 상태 머신, 패링(Clash), 강인도 관리

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/BPI_CombatEntity.h" // IBP_CombatEntity 인터페이스 포함
#include "BPC_InnerCombat.generated.h"

// Forward Declarations
class ACharacter;
class UAnimMontage;
class AActor;
class UBPC_TankMovement;
class UBP_RPGStats;

// 패링(Clash) 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClashOccurred, AActor*, actor, FVector, vector);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaggered, AActor*, actor, FVector, vector);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGuardToggled);

/**
 * BPC_InnerCombat
 * 
 * Inner World Knight의 전투 시스템을 담당하는 컴포넌트.
 * IBP_CombatEntity 인터페이스를 구현하여 표준화된 전투 상호작용을 제공.
 *
 * 역할:
 * 1. 전투 상태 머신 관리 (Idle, Attacking, Defending, Staggered, Dead)
 * 2. 가드(Guard) 및 패링(Clash)과 같은 전투 액션 처리
 * 3. 애니메이션(몽타주) 재생 및 관련 이벤트 처리
 *
 * 데이터:
 * - 강인도(Poise)를 포함한 모든 숫자 능력치는 BPC_RPGStats 컴포넌트에서 관리.
 * - 이 컴포넌트는 BPC_RPGStats를 참조하여 데이터를 읽고, 변경을 요청.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class UEFRAMEWORKTEST_API UBPC_InnerCombat : public UActorComponent, public IBP_CombatEntity
{
	GENERATED_BODY()

public:
	UBPC_InnerCombat();

	/**
	 * 가드 이동 속도 배율
	 * 
	 * Default: 0.5 (50%)
	 * 가드 중 실제 이동 속도 = MaxMoveSpeed * GuardMovePenalty
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Guard")
	float GuardMovePenalty;

	/**
	 * 패링(Clash) 가능 시간
	 * 
	 * Default: 0.2 (0.2초)
	 * 공격 시작 후 ClashWindowTime 동안 bCanClash = true.
	 * 이 시간 내에 적 무기와 충돌하면 패링 성공.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Clash")
	float ClashWindowTime;
	
	/**
	 * 패링 실패 시 받을 강인도 데미지
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Clash")
	float ClashPoiseDamage;

	/**
	 * 현재 패링 가능 여부
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Clash")
	bool bCanClash;

	/**
	 * 공격 몽타주 (경공격)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	UAnimMontage* AttackMontageLight;

	/**
	 * 공격 몽타주 (무거운 공격)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	UAnimMontage* AttackMontageHeavy;

	/**
	 * 스태거 몽타주
	 * 강인도가 0이 되었을 때 재생됨.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	UAnimMontage* StaggerMontage;

	/**
	 * 패링 스파크 파티클 이펙트
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
	class UParticleSystem* ClashSparkEffect;

	/**
	 * 패링 사운드 큐
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
	class USoundBase* ClashSound;

	// ==================== 이벤트 ====================

	/**
	 * 패링 발생 이벤트
	 * CheckClash() 내에서 bCanClash == true일 때 발생.
	 * 
	 * 매개변수:
	 * - AActor*: 충돌한 적 액터
	 * - FVector: 충돌 위치
	 */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnClashOccurred OnClashOccurred;

	/**
	 * 스태거 이벤트
	 * BPC_RPGStats로부터 Stagger 상태 진입을 통보받았을 때 발생.
	 * 
	 * 매개변수:
	 * - AActor*: 공격자 액터
	 * - FVector: 임팩트 방향
	 */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnStaggered OnStaggeredDelegate;
	
	/**
	 * 가드 토글 이벤트
	 * ToggleGuard() 호출 시 발생.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnGuardToggled OnGuardToggled;

protected:
	virtual void BeginPlay() override;

	// 소유자 캐릭터 캐시
	UPROPERTY()
	ACharacter* OwnerCharacter;

	// 소유자의 TankMovement 컴포넌트 캐시
	UPROPERTY()
	UBPC_TankMovement* TankMovementComponent;
	
	// 소유자의 RPGStats 컴포넌트 캐시
	UPROPERTY()
	UBP_RPGStats* RPGStatsComponent;

	// 클래시 윈도우 타이머 핸들
	FTimerHandle ClashWindowTimerHandle;

	// 현재 전투 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	EBSCombatState CurrentCombatState;
	
public:
	//~ Begin IBP_CombatEntity Interface
	virtual float GetCurrentPoise_Implementation() const override;
	virtual float GetMaxPoise_Implementation() const override;
	virtual void ReceivePoiseDamage_Implementation(float PoiseDamage, AActor* Instigator) override;
	virtual bool AttemptAttack_Implementation(bool bIsHeavy) override;
	virtual bool CheckClash_Implementation(AActor* OtherWeapon) override;
	virtual void ToggleGuard_Implementation(bool bEnable) override;
	virtual bool IsGuardActive_Implementation() const override;
	virtual EBSCombatState GetCurrentCombatState_Implementation() const override;
	virtual void OnStaggered_Implementation(FVector ImpactDirection) override;
	virtual void OnDeath_Implementation() override;
	virtual bool IsAlive_Implementation() const override;
	//~ End IBP_CombatEntity Interface

	/**
	 * IsInCombat
	 * 현재 활발한 전투 중인지 여부
	 * (Attacking 또는 Staggered 상태)
	 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInCombat() const;

protected:
	/**
	 * OnClashWindowEnd
	 * ClashWindow 타이머 완료 콜백
	 */
	void OnClashWindowEnd();

	/**
	 * SpawnClashEffect
	 * 패링 시각/음향 효과 생성
	 */
	void SpawnClashEffect(FVector Location);

	/**
	 * SetCombatState
	 * 전투 상태 변경
	 */
	void SetCombatState(EBSCombatState NewState);
	
	/**
	 * OnMontageEnded
	 * 모든 전투 몽타주가 끝났을 때 호출되는 함수
	 */
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	/**
	 * HandleStagger
	 * BPC_RPGStats로부터 스태거 이벤트를 받았을 때 호출됨
	 */
	UFUNCTION()
	void HandleStagger(FVector ImpactDirection);
};

