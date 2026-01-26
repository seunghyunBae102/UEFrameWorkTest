// Ghost in the Monitor - Inner World Item Data Asset
// DA_InnerItem.h
// Inner World의 무기, 소모품 등 아이템 데이터 정의

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_InnerItem.generated.h"

// Forward Declarations
class UAnimMontage;
class UStaticMesh;
class UTexture2D;
class USoundBase;

/**
 * Inner World Item 타입
 */
UENUM(BlueprintType)
enum class EInnerItemType : uint8
{
	Weapon_Melee = 0    UMETA(DisplayName = "Melee Weapon"),
	Weapon_Ranged = 1   UMETA(DisplayName = "Ranged Weapon"),
	Consumable_Potion = 2 UMETA(DisplayName = "Potion"),
	Consumable_Buff = 3 UMETA(DisplayName = "Buff Item"),
	Misc = 4            UMETA(DisplayName = "Miscellaneous")
};

/**
 * 무기 타입 (EInnerItemType == Weapon_Melee 또는 Weapon_Ranged일 때)
 */
UENUM(BlueprintType)
enum class EWeaponSubType : uint8
{
	Sword = 0       UMETA(DisplayName = "Sword"),
	Axe = 1         UMETA(DisplayName = "Axe"),
	Bow = 2         UMETA(DisplayName = "Bow"),
	Staff = 3       UMETA(DisplayName = "Staff"),
	Hammer = 4      UMETA(DisplayName = "Hammer")
};

/**
 * DA_InnerItem
 * 
 * Inner World의 모든 아이템(무기, 소모품 등) 데이터를 정의하는 Primary Data Asset.
 * 
 * 종류:
 * 1. 근접 무기 (Melee Weapon)
 *    - 검, 도끼, 망치 등
 *    - 공격 몽타주, 데미지, 범위 공격 반경 등
 * 
 * 2. 원거리 무기 (Ranged Weapon)
 *    - 활, 지팡이 등
 *    - 발사체 정보, 재장전 속도 등
 * 
 * 3. 소모품 (Consumable)
 *    - 포션(체력 회복), 버프(능력 향상) 등
 *    - 회복량, 버프 지속시간 등
 * 
 * 4. 기타 (Misc)
 *    - 중요하지 않은 아이템
 * 
 * 저장소:
 * /Content/InnerWorld/Items/ 폴더에 저장
 * 예: DA_Sword_Iron.uasset, DA_Potion_Health.uasset
 */
UCLASS(BlueprintType)
class UEFRAMEWORKTEST_API UDA_InnerItem : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UDA_InnerItem();

	// ==================== 기본 정보 ====================

	/**
	 * 아이템 이름
	 * 게임 내 표시 이름
	 * 
	 * 예: "철검", "생명력 물약", "불의 부적"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemInfo")
	FText DisplayName;

	/**
	 * 아이템 설명
	 * 아이템 상세 정보
	 * 
	 * 예: "견고한 철로 만든 기본 검. 적당한 공격력을 지님."
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemInfo")
	FText Description;

	/**
	 * 아이템 타입
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemInfo")
	EInnerItemType ItemType;

	/**
	 * 아이템 아이콘 (UI에 표시)
	 * 슬롯과 인벤토리에 표시되는 아이콘.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemInfo")
	UTexture2D* Icon;

	/**
	 * 아이템 무게 (kg)
	 * 
	 * Default: 0.0
	 * 장착 무기의 무게는 이동 속도에 영향을 줄 수 있음.
	 * 
	 * 예: 검(1.5kg), 도끼(2.0kg), 포션(0.1kg)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemInfo")
	float Weight;

	// ==================== 무기 정보 (Weapon_Melee, Weapon_Ranged) ====================

	/**
	 * 무기 서브타입
	 * (ItemType이 Weapon_Melee 또는 Weapon_Ranged일 때만 사용)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (EditCondition = "ItemType == EInnerItemType::Weapon_Melee || ItemType == EInnerItemType::Weapon_Ranged"))
	EWeaponSubType WeaponSubType;

	/**
	 * 무기 메시
	 * 3D 모델 메시.
	 * Character의 소켓에 부착됨.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (EditCondition = "ItemType == EInnerItemType::Weapon_Melee || ItemType == EInnerItemType::Weapon_Ranged"))
	UStaticMesh* WeaponMesh;

	/**
	 * 근접 공격 데미지 (Melee Weapon)
	 * 
	 * Default: 10.0
	 * 이 값은 공격 몽타주의 Notify에서 사용됨.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Melee", meta = (EditCondition = "ItemType == EInnerItemType::Weapon_Melee"))
	float MeleeDamage;

	/**
	 * 근접 공격 범위 (Melee Weapon)
	 * 
	 * Default: 100.0 (Unreal Units)
	 * 공격 판정 구 추적의 반지름.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Melee", meta = (EditCondition = "ItemType == EInnerItemType::Weapon_Melee"))
	float MeleeRange;

	/**
	 * 경공격 몽타주
	 * 경공격(Light Attack) 애니메이션.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Melee", meta = (EditCondition = "ItemType == EInnerItemType::Weapon_Melee"))
	UAnimMontage* AttackMontageLightAttack;

	/**
	 * 무거운 공격 몽타주
	 * 무거운 공격(Heavy Attack) 애니메이션.
	 * 
	 * 로드 시간이 더 길지만 데미지가 높음.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Melee", meta = (EditCondition = "ItemType == EInnerItemType::Weapon_Melee"))
	UAnimMontage* AttackMontageHeavyAttack;

	/**
	 * 원거리 공격 데미지 (Ranged Weapon)
	 * 
	 * Default: 15.0
	 * 발사체의 데미지.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged", meta = (EditCondition = "ItemType == EInnerItemType::Weapon_Ranged"))
	float RangedDamage;

	/**
	 * 발사체 클래스
	 * (Ranged Weapon)
	 * 
	 * 발사할 발사체 액터 클래스.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged", meta = (EditCondition = "ItemType == EInnerItemType::Weapon_Ranged"))
	TSubclassOf<class AActor> ProjectileClass;

	/**
	 * 재장전 속도 (Ranged Weapon)
	 * 
	 * Default: 1.0 (초)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged", meta = (EditCondition = "ItemType == EInnerItemType::Weapon_Ranged"))
	float ReloadSpeed;

	/**
	 * 조준 몽타주
	 * (Ranged Weapon)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged", meta = (EditCondition = "ItemType == EInnerItemType::Weapon_Ranged"))
	UAnimMontage* AimMontage;

	// ==================== 소모품 정보 (Consumable_Potion, Consumable_Buff) ====================

	/**
	 * 회복량 (Potion)
	 * 
	 * Default: 50.0 (체력 회복)
	 * (ItemType == Consumable_Potion일 때만 사용)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable|Potion", meta = (EditCondition = "ItemType == EInnerItemType::Consumable_Potion"))
	float HealAmount;

	/**
	 * 강인도 회복량 (Potion)
	 * 
	 * Default: 20.0
	 * 강인도(Poise)를 회복할 수도 있음.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable|Potion", meta = (EditCondition = "ItemType == EInnerItemType::Consumable_Potion"))
	float PoiseRecoveryAmount;

	/**
	 * 사용 애니메이션
	 * 약 마시는 애니메이션.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable|Potion", meta = (EditCondition = "ItemType == EInnerItemType::Consumable_Potion"))
	UAnimMontage* UseMontage;

	/**
	 * 버프 타입
	 * (Consumable_Buff)
	 * 
	 * 예: "StrengthBuff", "SpeedBuff", "DefenseBuff"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable|Buff", meta = (EditCondition = "ItemType == EInnerItemType::Consumable_Buff"))
	FName BuffType;

	/**
	 * 버프 지속 시간
	 * 
	 * Default: 10.0 (초)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable|Buff", meta = (EditCondition = "ItemType == EInnerItemType::Consumable_Buff"))
	float BuffDuration;

	/**
	 * 버프 강도 (배수)
	 * 
	 * Default: 1.5 (1.5배 능력 향상)
	 * 예: AttackPower * BuffPower
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable|Buff", meta = (EditCondition = "ItemType == EInnerItemType::Consumable_Buff"))
	float BuffPower;

	// ==================== 효과음/음향 ====================

	/**
	 * 착용 사운드 (무기 장착 시)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	USoundBase* EquipSound;

	/**
	 * 사용 사운드 (아이템 사용 시)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	USoundBase* UseSound;

	// ==================== 유틸리티 ====================

	/**
	 * IsWeapon
	 * 이 아이템이 무기인지 확인
	 */
	UFUNCTION(BlueprintCallable, Category = "ItemInfo")
	bool IsWeapon() const { return ItemType == EInnerItemType::Weapon_Melee || ItemType == EInnerItemType::Weapon_Ranged; }

	/**
	 * IsConsumable
	 * 이 아이템이 소모품인지 확인
	 */
	UFUNCTION(BlueprintCallable, Category = "ItemInfo")
	bool IsConsumable() const { return ItemType == EInnerItemType::Consumable_Potion || ItemType == EInnerItemType::Consumable_Buff; }

	/**
	 * IsMeleeWeapon
	 * 이 아이템이 근접 무기인지 확인
	 */
	UFUNCTION(BlueprintCallable, Category = "ItemInfo")
	bool IsMeleeWeapon() const { return ItemType == EInnerItemType::Weapon_Melee; }

	/**
	 * IsRangedWeapon
	 * 이 아이템이 원거리 무기인지 확인
	 */
	UFUNCTION(BlueprintCallable, Category = "ItemInfo")
	bool IsRangedWeapon() const { return ItemType == EInnerItemType::Weapon_Ranged; }

	/**
	 * IsPotion
	 * 이 아이템이 포션인지 확인
	 */
	UFUNCTION(BlueprintCallable, Category = "ItemInfo")
	bool IsPotion() const { return ItemType == EInnerItemType::Consumable_Potion; }

	/**
	 * GetPrimaryAssetId
	 * PrimaryDataAsset ID 조회
	 * (Unreal의 Asset Registry에서 사용)
	 */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
