// Ghost in the Monitor - Inner World Quick Slot System
// BPC_QuickSlot.cpp
// Implementation

#include "Inner/BPC_QuickSlot.h"
#include "Data/DA_InnerItem.h"
#include "GameFramework/Character.h"

UBPC_QuickSlot::UBPC_QuickSlot()
	: CurrentSlotIndex(0)
	, OwnerCharacter(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBPC_QuickSlot::BeginPlay()
{
	Super::BeginPlay();

	// 소유자 캐릭터 캐시
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_QuickSlot: Owner is not a Character!"));
		return;
	}

	// 슬롯 초기화
	InitializeSlots();
}

bool UBPC_QuickSlot::UseSlot()
{
	// 현재 슬롯의 아이템 조회
	UDA_InnerItem* Item = GetCurrentItem();
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_QuickSlot::UseSlot - Slot %d is empty"), CurrentSlotIndex);
		return false;
	}

	// 아이템 타입에 따라 처리
	if (Item->IsWeapon())
	{
		EquipWeapon(Item);
	}
	else if (Item->IsConsumable())
	{
		ConsumeItem(Item);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_QuickSlot::UseSlot - Item type not supported: %d"), (int32)Item->ItemType);
		return false;
	}

	// 이벤트 발생
	OnItemUsed.Broadcast(CurrentSlotIndex, Item);

	UE_LOG(LogTemp, Log, TEXT("BPC_QuickSlot::UseSlot - Used item from slot %d"), CurrentSlotIndex);
	return true;
}

void UBPC_QuickSlot::ChangeSlot(int32 NewSlotIndex)
{
	// 범위 클램핑
	int32 PrevSlotIndex = CurrentSlotIndex;
	CurrentSlotIndex = FMath::Clamp(NewSlotIndex, 0, MAX_SLOT_INDEX);

	if (CurrentSlotIndex == PrevSlotIndex)
	{
		return; // 변경 없음
	}

	// 이벤트 발생
	UDA_InnerItem* CurrentItem = GetCurrentItem();
	OnSlotChanged.Broadcast(CurrentSlotIndex, CurrentItem);

	UE_LOG(LogTemp, Log, TEXT("BPC_QuickSlot::ChangeSlot - Changed from slot %d to %d (UI: %d->%d)"), 
		PrevSlotIndex, CurrentSlotIndex, PrevSlotIndex + 1, CurrentSlotIndex + 1);
}

UDA_InnerItem* UBPC_QuickSlot::GetCurrentItem() const
{
	if (Slots.IsValidIndex(CurrentSlotIndex))
	{
		return Slots[CurrentSlotIndex];
	}
	return nullptr;
}

UDA_InnerItem* UBPC_QuickSlot::GetSlotItem(int32 SlotIndex) const
{
	if (Slots.IsValidIndex(SlotIndex))
	{
		return Slots[SlotIndex];
	}
	return nullptr;
}

bool UBPC_QuickSlot::SetSlotItem(int32 SlotIndex, UDA_InnerItem* Item)
{
	// 범위 검증
	if (!Slots.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_QuickSlot::SetSlotItem - Invalid slot index: %d"), SlotIndex);
		return false;
	}

	Slots[SlotIndex] = Item;

	if (Item)
	{
		UE_LOG(LogTemp, Log, TEXT("BPC_QuickSlot::SetSlotItem - Set slot %d to item: %s"), SlotIndex, *Item->DisplayName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("BPC_QuickSlot::SetSlotItem - Cleared slot %d"), SlotIndex);
	}

	return true;
}

bool UBPC_QuickSlot::IsSlotEmpty(int32 SlotIndex) const
{
	if (!Slots.IsValidIndex(SlotIndex))
	{
		return true;
	}
	return Slots[SlotIndex] == nullptr;
}

void UBPC_QuickSlot::ClearAllSlots()
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		Slots[i] = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("BPC_QuickSlot::ClearAllSlots - All slots cleared"));
}

void UBPC_QuickSlot::InitializeSlots()
{
	// 슬롯 배열 크기 설정
	Slots.SetNum(SLOT_COUNT);

	// 모든 슬롯을 nullptr로 초기화
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		Slots[i] = nullptr;
	}

	CurrentSlotIndex = 0;

	UE_LOG(LogTemp, Log, TEXT("BPC_QuickSlot::InitializeSlots - Initialized %d slots"), SLOT_COUNT);
}

void UBPC_QuickSlot::EquipWeapon(UDA_InnerItem* WeaponItem)
{
	if (!WeaponItem || !WeaponItem->IsWeapon())
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_QuickSlot::EquipWeapon - Item is not a weapon"));
		return;
	}

	// Owner(ABP_InnerKnight)의 EquipWeapon() 메서드 호출
	// (ABP_InnerKnight 클래스에서 구현됨)
	if (OwnerCharacter)
	{
		// 현재는 로그만 출력
		// 실제 무기 장착은 ABP_InnerKnight의 EquipWeapon() 메서드에서 처리
		UE_LOG(LogTemp, Log, TEXT("BPC_QuickSlot::EquipWeapon - Equipped weapon: %s"), *WeaponItem->DisplayName.ToString());
	}
}

void UBPC_QuickSlot::ConsumeItem(UDA_InnerItem* ConsumableItem)
{
	if (!ConsumableItem || !ConsumableItem->IsConsumable())
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_QuickSlot::ConsumeItem - Item is not consumable"));
		return;
	}

	// 소모품 사용 처리
	if (ConsumableItem->IsPotion())
	{
		// 포션 사용
		float HealAmount = ConsumableItem->HealAmount;
		float PoiseRecovery = ConsumableItem->PoiseRecoveryAmount;

		UE_LOG(LogTemp, Log, TEXT("BPC_QuickSlot::ConsumeItem - Used potion: %s (Heal=%.1f, Poise=%.1f)"), 
			*ConsumableItem->DisplayName.ToString(), HealAmount, PoiseRecovery);

		// 실제 회복 로직은 ABP_InnerKnight 또는 Health System에서 처리
	}
	else if (ConsumableItem->ItemType == EInnerItemType::Consumable_Buff)
	{
		// 버프 아이템 사용
		FName BuffType = ConsumableItem->BuffType;
		float Duration = ConsumableItem->BuffDuration;
		float Power = ConsumableItem->BuffPower;

		UE_LOG(LogTemp, Log, TEXT("BPC_QuickSlot::ConsumeItem - Applied buff: %s (Type=%s, Duration=%.1f, Power=%.2f)"), 
			*ConsumableItem->DisplayName.ToString(), *BuffType.ToString(), Duration, Power);

		// 실제 버프 적용 로직은 별도 Buff System에서 처리
	}

	// 소모품 사용 후 슬롯에서 제거
	SetSlotItem(CurrentSlotIndex, nullptr);
}
