// Ghost in the Monitor - Inner World Item Data Asset
// DA_InnerItem.cpp
// Implementation

#include "Data/DA_InnerItem.h"

UDA_InnerItem::UDA_InnerItem()
	: DisplayName(FText::FromString(TEXT("New Item")))
	, Description(FText::FromString(TEXT("Item Description")))
	, ItemType(EInnerItemType::Misc)
	, Icon(nullptr)
	, Weight(0.0f)
	, WeaponSubType(EWeaponSubType::Sword)
	, WeaponMesh(nullptr)
	, MeleeDamage(10.0f)
	, MeleeRange(100.0f)
	, AttackMontageLightAttack(nullptr)
	, AttackMontageHeavyAttack(nullptr)
	, RangedDamage(15.0f)
	, ProjectileClass(nullptr)
	, ReloadSpeed(1.0f)
	, AimMontage(nullptr)
	, HealAmount(50.0f)
	, PoiseRecoveryAmount(20.0f)
	, UseMontage(nullptr)
	, BuffType(NAME_None)
	, BuffDuration(10.0f)
	, BuffPower(1.5f)
	, EquipSound(nullptr)
	, UseSound(nullptr)
{
}

FPrimaryAssetId UDA_InnerItem::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("InnerItem"), GetFName());
}
