// GHOST IN THE MONITOR - Tactical Weapon Component
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/DA_AmmoType.h"
#include "Data/DA_WeaponPart.h"
#include "BPC_MagLogistic.h"
#include "BPC_TacticalWeapon.generated.h"

/**
 * 무기 작동 상태
 */
UENUM(BlueprintType)
enum class EWeaponMalfunctionState : uint8
{
	Normal = 0,          // 정상 작동
	StovePipe = 1,       // 배출불량 (탄피가 배출되지 않고 노리쇠에 걸림)
	DoubleFeed = 2,      // 급탄불량 (탄이 2개 이상 장전됨)
	Dud = 3              // 불발 (탄약이 터지지 않음)
};

/**
 * BPC_TacticalWeapon - 총기 작동 메커니즘
 * 
 * 역할:
 * - 격발 시뮬레이션 (Trigger)
 * - 잼(Jamming) 판정
 * - 노리쇠 왕복 (Bolt Cycling)
 * - 탄창 관리와 상호작용
 * 
 * 규칙:
 * - Tick 비활성화 (이벤트/함수 호출로만 작동)
 * - 탄창(BPC_MagLogistic)을 참조하지만, 직접 조작하지 않음 (Interface/Delegate로만 통신)
 * - 신뢰도 감소는 매번 격발할 때 발생
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UEFRAMEWORKTEST_API UBP_TacticalWeapon : public UActorComponent
{
	GENERATED_BODY()

public:
	UBP_TacticalWeapon();

	virtual void BeginPlay() override;

	// ==================== Properties ====================

	// 현재 총기 신뢰도 (0.0 ~ 1.0)
	// 격발할 때마다 감소. 잼 발생 확률에 영향.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	float CurrentReliability = 1.0f;

	// 초기 신뢰도 (리셋용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float BaseReliability = 1.0f;

	// 격발마다 신뢰도 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float ReliabilityDecayPerShot = 0.01f;

	// 현재 무기 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponMalfunctionState MalfunctionState = EWeaponMalfunctionState::Normal;

	// 약실에 들어있는 탄 1발
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	UDA_AmmoType* ChamberedRound = nullptr;

	// 현재 장착한 탄창
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UBP_MagLogistic* MagazineComponent = nullptr;

	// 각 무기별 부품 (스탯 보정용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parts")
	TArray<UDA_WeaponPart*> InstalledParts;

	// ==================== Events ====================

	// 격발 시도 (성공/실패)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFired, bool, bSuccess, EWeaponMalfunctionState, Malfunction, float, TimeToFix);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFired OnFired;

	// 노리쇠가 왕복했을 때 (차탄 장전 완료)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoltCycled, bool, bSuccess);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBoltCycled OnBoltCycled;

	// 탄이 없을 때 (클릭음)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEmptyClick);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEmptyClick OnEmptyClick;

	// 잼이 발생했을 때
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMalfunctionOccurred, EWeaponMalfunctionState, MalfunctionType, float, TimeToFixMs);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMalfunctionOccurred OnMalfunctionOccurred;

	// ==================== Methods ====================

	/**
	 * 트리거 당김 (격발 시도)
	 * @param OutTimeToFix: 잼이 발생했을 경우 해결 시간 (밀리초)
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void PullTrigger(float& OutTimeToFix);

	/**
	 * 노리쇠 왕복 (탄피 배출 + 차탄 장전)
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool CycleBolt();

	/**
	 * 기능 고장 해결
	 * (예: 노리쇠를 여러 번 당겨서 걸린 탄 빼내기)
	 * @return 해결 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool FixMalfunction();

	/**
	 * 신뢰도 초기화 (총기 정비 후)
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void RestoreReliability();

	/**
	 * 약실에 있는 탄 정보 조회
	 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	UDA_AmmoType* GetChamberedRound() const { return ChamberedRound; }

	/**
	 * 현재 신뢰도 조회 (0.0 ~ 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetReliability() const { return CurrentReliability; }

	/**
	 * 무기가 발사 가능한 상태인지 확인
	 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanFire() const;

private:
	/**
	 * 잼 발생 판정 (RNG)
	 * @param Ammo: 발사할 탄약
	 * @param OutMalfunctionState: 어떤 종류의 잼이 발생했는지
	 * @return 잼 발생 여부
	 */
	bool DetermineMalfunction(UDA_AmmoType* Ammo, EWeaponMalfunctionState& OutMalfunctionState);

	/**
	 * 잼 해결 시간 계산 (밀리초)
	 * @param MalfunctionType: 잼의 종류
	 */
	float GetFixTimeForMalfunction(EWeaponMalfunctionState MalfunctionType) const;

	/**
	 * 부품 보정치의 총합 계산
	 * @param ModifierType: 어떤 종류의 보정을 계산할 것인가 (Reliability, Recoil 등)
	 */
	float GetTotalPartModifier(const FString& ModifierType) const;
};
