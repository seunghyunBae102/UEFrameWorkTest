// Ghost in the Monitor - Inner World Quick Slot System
// BPC_QuickSlot.h
// 1~5번 슬롯 아이템 데이터 관리

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BPC_QuickSlot.generated.h"

// Forward Declarations
class UDA_InnerItem;
class ACharacter;

// 슬롯 변경 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlotChanged, int32, slotidx, UDA_InnerItem*,item);

// 아이템 사용 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUsed, int32,slotidx, UDA_InnerItem*,item);

/**
 * BPC_QuickSlot
 * 
 * Knight 캐릭터의 Quick Slot 시스템을 담당.
 * 
 * 기능:
 * 1. 5개 슬롯 관리 (인덱스 0~4, UI에서는 1~5로 표시)
 * 2. 슬롯 전환 (E/D 키 또는 마우스휠)
 * 3. 아이템 사용 (Space 키)
 *    - 무기: EquipWeapon() 호출
 *    - 소모품: ConsumeItem() 호출
 * 4. 인벤토리와 동기화
 * 
 * 통합:
 * - ABP_InnerKnight의 자식 컴포넌트
 * - 무기 장착/교체 시스템과 연동
 * - 소비 아이템(물약) 사용 시스템과 연동
 */
UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class UEFRAMEWORKTEST_API UBPC_QuickSlot : public UActorComponent
{
	GENERATED_BODY()

public:
	UBPC_QuickSlot();

	/**
	 * 슬롯 배열 (크기: 5)
	 * 
	 * 인덱스:
	 * - 0 (UI: 1) - 주 무기
	 * - 1 (UI: 2) - 보조 무기
	 * - 2 (UI: 3) - 물약/회복 아이템
	 * - 3 (UI: 4) - 강화 아이템
	 * - 4 (UI: 5) - 예비 슬롯
	 * 
	 * nullptr이면 슬롯이 비어있음.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuickSlot")
	TArray<UDA_InnerItem*> Slots;

	/**
	 * 현재 선택된 슬롯 인덱스 (0~4)
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuickSlot")
	int32 CurrentSlotIndex;

	/**
	 * 슬롯이 변경되었을 때 발생하는 이벤트
	 * 
	 * 매개변수:
	 * - int32: 현재 슬롯 인덱스 (0~4)
	 * - UDA_InnerItem*: 슬롯에 있는 아이템 (nullptr이면 비어있음)
	 */
	UPROPERTY(BlueprintAssignable, Category = "QuickSlot|Events")
	FOnSlotChanged OnSlotChanged;

	/**
	 * 아이템이 사용되었을 때 발생하는 이벤트
	 * 
	 * 매개변수:
	 * - int32: 사용된 아이템의 슬롯 인덱스 (0~4)
	 * - UDA_InnerItem*: 사용된 아이템
	 */
	UPROPERTY(BlueprintAssignable, Category = "QuickSlot|Events")
	FOnItemUsed OnItemUsed;

protected:
	virtual void BeginPlay() override;

	// 소유자 캐릭터 캐시
	UPROPERTY()
	ACharacter* OwnerCharacter;

public:
	/**
	 * UseSlot
	 * 현재 슬롯의 아이템 사용
	 * 
	 * 로직:
	 * 1. CurrentSlotIndex의 아이템 조회
	 * 2. 아이템이 nullptr이면 조기 반환
	 * 3. 아이템 타입에 따라:
	 *    a) Weapon: EquipWeapon(Item)
	 *    b) Consumable: ConsumeItem(Item)
	 *    c) Default: UE_LOG Warning
	 * 4. OnItemUsed 이벤트 발생
	 * 
	 * @return 아이템이 사용되었는지 여부
	 * 
	 * 입력: Space Key
	 * 
	 * 예제:
	 *   if (InputComponent->IsActionPressed("Use"))
	 *   {
	 *       QuickSlot->UseSlot();
	 *   }
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	bool UseSlot();

	/**
	 * ChangeSlot
	 * 현재 선택 슬롯 변경
	 * 
	 * 로직:
	 * 1. NewSlotIndex를 0~4 범위로 클램핑
	 * 2. CurrentSlotIndex = NewSlotIndex
	 * 3. OnSlotChanged 이벤트 발생
	 * 
	 * @param NewSlotIndex - 새 슬롯 인덱스 (0~4)
	 * 
	 * 입력: E/D 키 또는 마우스휠
	 * 
	 * 예제:
	 *   if (InputComponent->IsActionPressed("NextSlot"))
	 *   {
	 *       QuickSlot->ChangeSlot(CurrentSlotIndex + 1);
	 *   }
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void ChangeSlot(int32 NewSlotIndex);

	/**
	 * GetCurrentItem
	 * 현재 선택된 슬롯의 아이템 조회
	 * 
	 * @return 현재 슬롯의 아이템 (nullptr이면 비어있음)
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	UDA_InnerItem* GetCurrentItem() const;

	/**
	 * GetSlotItem
	 * 특정 인덱스 슬롯의 아이템 조회
	 * 
	 * @param SlotIndex - 슬롯 인덱스 (0~4)
	 * @return 슬롯의 아이템 (nullptr이면 비어있음)
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	UDA_InnerItem* GetSlotItem(int32 SlotIndex) const;

	/**
	 * SetSlotItem
	 * 특정 인덱스 슬롯에 아이템 설정
	 * 
	 * @param SlotIndex - 슬롯 인덱스 (0~4)
	 * @param Item - 설정할 아이템 (nullptr이면 슬롯 비움)
	 * @return 성공 여부
	 * 
	 * 예제:
	 *   QuickSlot->SetSlotItem(0, NewWeapon);
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	bool SetSlotItem(int32 SlotIndex, UDA_InnerItem* Item);

	/**
	 * GetCurrentSlotIndex
	 * 현재 슬롯 인덱스 조회 (0~4)
	 * UI에서 표시할 때는 +1 하여 1~5로 표시
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	/**
	 * IsSlotEmpty
	 * 특정 슬롯이 비어있는지 확인
	 * 
	 * @param SlotIndex - 슬롯 인덱스 (0~4)
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	bool IsSlotEmpty(int32 SlotIndex) const;

	/**
	 * ClearAllSlots
	 * 모든 슬롯 비우기
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void ClearAllSlots();

	/**
	 * GetSlotCount
	 * 슬롯 개수 조회 (항상 5)
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	int32 GetSlotCount() const { return Slots.Num(); }

	/**
	 * InitializeSlots
	 * 슬롯 초기화
	 * BeginPlay에서 자동으로 호출됨.
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void InitializeSlots();

protected:
	/**
	 * EquipWeapon
	 * 무기 장착 (내부 헬퍼 함수)
	 * 
	 * @param WeaponItem - 장착할 무기 아이템
	 */
	void EquipWeapon(UDA_InnerItem* WeaponItem);

	/**
	 * ConsumeItem
	 * 소모품 사용 (내부 헬퍼 함수)
	 * 
	 * @param ConsumableItem - 사용할 소모품 아이템
	 */
	void ConsumeItem(UDA_InnerItem* ConsumableItem);

private:
	/** 슬롯 수 (상수) */
	static constexpr int32 SLOT_COUNT = 5;

	/** 슬롯 인덱스 최대값 (4 = SLOT_COUNT - 1) */
	static constexpr int32 MAX_SLOT_INDEX = SLOT_COUNT - 1;
};
