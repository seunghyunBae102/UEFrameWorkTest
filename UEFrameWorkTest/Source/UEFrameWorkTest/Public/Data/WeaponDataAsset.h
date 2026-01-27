// Copyright 2024, Korstian Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDataAsset.generated.h"

// Forward declaration
class ABaseWeapon;

/**
 * @enum EWeaponType
 * @brief 무기의 종류를 구분하기 위한 열거형입니다.
 * 애니메이션 레이어, 스탯 보너스 등 무기 타입별로 다른 로직을 적용하는 데 사용될 수 있습니다.
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None,
	Sword,
	Axe,
	Spear,
	Gun
};

/**
 * @struct FAttackData
 * @brief 단일 공격 하나에 대한 모든 데이터를 정의하는 구조체입니다.
 * 약공격/강공격 콤보 배열에 이 구조체들이 순서대로 포함됩니다.
 */
USTRUCT(BlueprintType)
struct FAttackData
{
	GENERATED_BODY()

	/** @brief 이 공격에 재생될 애니메이션 몽타주입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<class UAnimMontage> Montage = nullptr;

	/** @brief 이 공격의 대미지 배율입니다. 캐릭터의 최종 공격력(AttackPower)에 곱해져 실제 피해량을 계산합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float DamageMultiplier = 1.0f;

	/** @brief 이 공격을 수행하는 데 필요한 스태미나 소모량입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float StaminaCost = 10.0f;

	/** @brief 적에게 입히는 경직(Poise) 피해량입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float PoiseDamage = 10.0f;
};

/**
 * @class UWeaponDataAsset
 * @brief 무기 하나의 모든 속성을 정의하는 데이터 애셋입니다.
 * 이 애셋을 사용함으로써, 코드를 변경하지 않고 데이터 수정만으로 새로운 무기를 추가하거나 밸런스를 조정하는 '데이터 주도' 설계를 가능하게 합니다.
 */
UCLASS(BlueprintType)
class UEFRAMEWORKTEST_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** @brief EquipmentComponent가 스폰할 무기의 스태틱 메쉬입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Info")
	TObjectPtr<UStaticMesh> WeaponMesh;

	/** @brief 캐릭터의 스켈레탈 메쉬에 무기를 부착할 소켓의 이름입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Info")
	FName SocketName = "hand_r_socket";

	/** @brief 이 무기의 타입입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Info")
	EWeaponType WeaponType = EWeaponType::None;

	/** @brief 약공격 콤보를 정의하는 FAttackData의 배열입니다. 배열의 순서가 콤보의 순서가 됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attacks")
	TArray<FAttackData> LightAttacks;

	/** @brief 강공격 콤보를 정의하는 FAttackData의 배열입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attacks")
	TArray<FAttackData> HeavyAttacks;

	/** @brief 패리 애니메이션 몽타주입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	TObjectPtr<class UAnimMontage> ParryMontage;

	/** @brief 가드(막기) 시 피해 감소율입니다. (0.0: 모든 피해 받음, 1.0: 모든 피해 무시) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BlockMitigation = 0.5f;

	/** @brief 가드(막기) 성공 시 소모되는 스태미나입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	float GuardStaminaCost = 5.0f;

	/** @brief 패링 애니메이션 내에서 실제 패링 판정이 시작되는 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	float ParryWindowStart = 0.0f;

	/** @brief 패링 애니메이션 내에서 실제 패링 판정이 끝나는 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	float ParryWindowEnd = 0.0f;
};