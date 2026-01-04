// GHOST IN THE MONITOR - Magazine Actor Base
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BPC_MagLogistic.h"
#include "AMagazineBase.generated.h"

/**
 * AMagazineBase - Outer World의 탄창 물리 액터
 * 
 * 역할:
 * - 탄창을 독립적인 액터로 표현 (책상 위에 놓인 물리 오브젝트)
 * - 플레이어가 집어들 수 있고, 버릴 수 있는 아이템
 * - BPC_MagLogistic 컴포넌트를 포함하여 탄약 관리
 * - 다른 탄창과 물리적 상호작용 가능
 * 
 * 아키텍처:
 * - StaticMeshComponent: 탄창 3D 모델
 * - BPC_MagLogistic: 탄약 데이터 및 물리 시뮬레이션
 * - CollisionComponent: 물리적 상호작용 (힘 적용 가능)
 * - SceneComponent: 기본 루트 컴포넌트
 * 
 * 규칙:
 * - 액터 기반이므로 월드에 자유롭게 배치 가능
 * - 플레이어 인벤토리에 들어갔다가 다시 나올 수 있음
 * - Physics enabled 가능 (시뮬레이션, 드롭)
 * - 다른 액터(총기)에 부착 가능 (Magazine Well)
 */
UCLASS()
class UEFRAMEWORKTEST_API AMagazineBase : public AActor
{
	GENERATED_BODY()

public:
	AMagazineBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ==================== Components ====================

	// 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* RootSceneComp;

	// 탄창 3D 모델
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComponent;

	// 탄약 관리 로직
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBP_MagLogistic* MagLogisticComponent;

	// 물리 충돌 (탄창을 집어들거나 부딪힐 때 사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionComponent;

	// ==================== Properties ====================

	// 탄창 타입 (9mm, 5.56mm 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
	FName MagazineType = TEXT("9x19mm");

	// 탄창 캐퍼시티 (최대 탄약 개수)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
	int32 MaxCapacity = 30;

	// 탄창이 부착된 부모 액터 (총기 또는 None)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attachment")
	class AActor* AttachedToWeapon = nullptr;

	// 물리 시뮬레이션 활성화 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Physics")
	bool bIsSimulatingPhysics = false;

	// 플레이어 인벤토리에 있는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsInInventory = false;

	// 탄창 ID (저장/로드용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FName MagazineID = TEXT("MAG_Unknown");

	// ==================== Events ====================

	// 탄약이 추가되었을 때 (MagLogisticComponent로부터 전파)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMagazineBulletAdded, int32, NewAmmoCount, float, NewWeight);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMagazineBulletAdded OnMagazineBulletAdded;

	// 탄약이 제거되었을 때
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMagazineBulletRemoved, int32, NewAmmoCount, float, NewWeight);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMagazineBulletRemoved OnMagazineBulletRemoved;

	// 탄창이 부착되었을 때
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMagazineAttached, AActor*, WeaponActor);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMagazineAttached OnMagazineAttached;

	// 탄창이 분리되었을 때
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMagazineDetached);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMagazineDetached OnMagazineDetached;

	// 인벤토리에 들어갔을 때
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTakenToInventory);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTakenToInventory OnTakenToInventory;

	// 인벤토리에서 나왔을 때 (월드에 드롭됨)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDroppedFromInventory, FVector, DropLocation);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDroppedFromInventory OnDroppedFromInventory;

	// ==================== Methods ====================

	/**
	 * 탄창 초기화 (특정 탄약으로 채우기)
	 * @param AmmoType: 넣을 탄약 타입
	 * @param AmmoCount: 몇 발 넣을지
	 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	void InitializeMagazine(class UDA_AmmoType* AmmoType, int32 AmmoCount);

	/**
	 * 탄창을 특정 액터에 부착 (총기 부착)
	 * @param WeaponActor: 부착할 무기 액터
	 * @param SocketName: 소켓 이름 (예: "MagazineWell")
	 * @return 부착 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	bool AttachToWeapon(AActor* WeaponActor, const FName& SocketName = TEXT("MagazineWell"));

	/**
	 * 탄창을 무기에서 분리 (및 월드에 드롭)
	 * @return 분리 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	bool DetachFromWeapon();

	/**
	 * 탄창을 플레이어 인벤토리로 가져가기 (숨김 처리)
	 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	void TakeToInventory();

	/**
	 * 탄창을 인벤토리에서 드롭 (월드에 다시 배치, 물리 활성화)
	 * @param DropLocation: 드롭될 위치
	 * @param DropVelocity: 드롭 시 초기 속도 (선택사항)
	 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	void DropFromInventory(FVector DropLocation, FVector DropVelocity = FVector::ZeroVector);

	/**
	 * 탄창의 현재 탄약 개수 조회
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Magazine")
	int32 GetAmmoCount() const;

	/**
	 * 탄창의 현재 무게 조회 (그램)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Magazine")
	float GetCurrentWeight() const;

	/**
	 * 탄창이 비어있는지 확인
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Magazine")
	bool IsEmpty() const;

	/**
	 * 탄창이 가득 찼는지 확인
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Magazine")
	bool IsFull() const;

	/**
	 * 탄창이 특정 무기에 부착되어 있는지 확인
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Magazine")
	bool IsAttachedToWeapon() const { return AttachedToWeapon != nullptr; }

	/**
	 * 탄약 추가 (복합 탄약 로드)
	 * @param Ammo: 추가할 탄약 에셋
	 * @param Count: 몇 발 추가할지
	 * @return 실제로 추가된 탄약 개수
	 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	int32 AddAmmo(class UDA_AmmoType* Ammo, int32 Count = 1);

	/**
	 * 탄약 소모 (총 발사 시 호출)
	 * @param OutAmmo: 꺼낸 탄약 에셋
	 * @return 탄약을 꺼냈는지 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	bool ConsumeAmmo(class UDA_AmmoType*& OutAmmo);

	/**
	 * 탄창 비우기
	 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	void EmptyMagazine();

	/**
	 * 탄창의 무게에 따른 상태 반환 (시각화용)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Magazine")
	EMagazineWeightStatus GetWeightStatus() const;

	/**
	 * 물리 시뮬레이션 활성화/비활성화
	 * @param bEnable: 활성화 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics")
	void SetPhysicsSimulation(bool bEnable);

private:
	/**
	 * 탄창의 시각적 표현 업데이트 (무게에 따른 메시 변경 등)
	 */
	void UpdateVisuals();

	/**
	 * MagLogisticComponent의 이벤트를 자신의 이벤트로 전파
	 */
	void BindMagLogisticEvents();

	/**
	 * MagLogistic 탄약 추가 콜백
	 */
	UFUNCTION()
	void OnMagLogisticBulletInserted(class UDA_AmmoType* InsertedAmmo, int32 CurrentCount);

	/**
	 * MagLogistic 탄약 제거 콜백
	 */
	UFUNCTION()
	void OnMagLogisticBulletRemoved(class UDA_AmmoType* RemovedAmmo, int32 CurrentCount);
};
