// GHOST IN THE MONITOR - Magazine Logistics Component
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/DA_AmmoType.h"
#include "BPC_MagLogistic.generated.h"

/**
 * 탄창 무게 상태 (정확한 숫자가 아닌 대략적 상태)
 */
UENUM(BlueprintType)
enum class EMagazineWeightStatus : uint8
{
	Empty = 0,   // 비어있음
	Light = 1,   // 가벼움 (1~3발)
	Medium = 2,  // 중간 (4~7발)
	Heavy = 3,   // 무거움 (8~15발)
	Full = 4     // 가득 참 (최대 용량)
};

/**
 * BPC_MagLogistic - 탄창 물리 시뮬레이션
 * 
 * 역할:
 * - 탄의 스택 관리 (배열)
 * - 삽탄 시간 계산
 * - 탄창 무게 추정
 * - 발화 불량 확률 추적
 * 
 * 규칙:
 * - Tick은 비활성화 (타이머/이벤트 기반 작동)
 * - Cast 금지 (Interface로만 통신)
 * - 상태 변화는 Event Dispatcher로 전파
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UEFRAMEWORKTEST_API UBP_MagLogistic : public UActorComponent
{
	GENERATED_BODY()

public:
	UBP_MagLogistic();

	virtual void BeginPlay() override;

	// ==================== Properties ====================

	// 현재 탄창에 들어있는 탄알들의 배열 (Stack 구조)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magazine")
	TArray<UDA_AmmoType*> LoadedAmmo;

	// 스프링 장력 (0.0 ~ 1.0)
	// 탄이 꽉 찰수록 1.0에 가까움. 삽탄 시간이 오래 걸림.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magazine")
	float SpringTension = 0.0f;

	// 최대 용량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
	int32 Capacity = 30;

	// 각 탄환마다 소요되는 삽탄 시간 (초, 기본값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
	float BaseInsertTimePerBullet = 0.1f;

	// ==================== Events ====================

	// 탄이 추가되었을 때 발생
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBulletInserted, UDA_AmmoType*, InsertedAmmo, int32, CurrentCount);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBulletInserted OnBulletInserted;

	// 탄이 제거되었을 때 발생
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBulletRemoved, UDA_AmmoType*, RemovedAmmo, int32, CurrentCount);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBulletRemoved OnBulletRemoved;

	// 탄창이 가득 찼을 때 발생
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMagazineFull);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMagazineFull OnMagazineFull;

	// 탄창이 비었을 때 발생
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMagazineEmpty);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMagazineEmpty OnMagazineEmpty;

	// ==================== Methods ====================

	/**
	 * 탄알 하나를 탄창에 삽입
	 * @param Ammo: 삽입할 탄약 데이터 에셋
	 * @param OutInsertTime: 소요된 삽탄 시간 (초)
	 * @return 삽입 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	bool InsertBullet(UDA_AmmoType* Ammo, float& OutInsertTime);

	/**
	 * 탄알 하나를 탄창에서 제거 (Pop)
	 * @param OutRemovedAmmo: 제거된 탄약 데이터
	 * @return 제거 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	bool RemoveBullet(UDA_AmmoType*& OutRemovedAmmo);

	/**
	 * 탄창의 무게에 기반한 대략적 상태 반환
	 * (정확한 탄 개수가 아님 - 무게로만 추정)
	 */
	UFUNCTION(BlueprintPure, Category = "Magazine")
	EMagazineWeightStatus EstimateWeightStatus() const;

	/**
	 * 현재 탄약 개수 반환
	 */
	UFUNCTION(BlueprintPure, Category = "Magazine")
	int32 GetAmmoCount() const { return LoadedAmmo.Num(); }

	/**
	 * 탄창의 현재 무게 계산 (그램 단위)
	 */
	UFUNCTION(BlueprintPure, Category = "Magazine")
	float GetCurrentWeight() const;

	/**
	 * 탄창 비우기
	 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	void ClearMagazine();

	/**
	 * 탄창이 가득 찼는지 확인
	 */
	UFUNCTION(BlueprintPure, Category = "Magazine")
	bool IsFull() const { return LoadedAmmo.Num() >= Capacity; }

	/**
	 * 탄창이 비었는지 확인
	 */
	UFUNCTION(BlueprintPure, Category = "Magazine")
	bool IsEmpty() const { return LoadedAmmo.Num() == 0; }

private:
	/**
	 * 스프링 장력 재계산
	 */
	void RecalculateSpringTension();

	/**
	 * 삽탄 시간 계산 (스프링 장력에 따라 변함)
	 */
	float CalculateInsertTime(int32 BulletCountBefore) const;
};
