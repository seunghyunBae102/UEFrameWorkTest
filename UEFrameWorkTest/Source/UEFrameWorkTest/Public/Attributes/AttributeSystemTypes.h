// Copyright 2024, Korstian Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/DataTable.h"
#include "AttributeSystemTypes.generated.h"

/**
 * @enum EAttributeType
 * @brief 캐릭터가 보유할 수 있는 모든 스탯의 종류를 정의합니다.
 * 각 스탯은 게임플레이 로직의 다양한 부분(예: 생명력, 공격력, 이동 속도)에 영향을 미칩니다.
 */
UENUM(BlueprintType)
enum class EAttributeType : uint8
{
	// --- Vitals (생명 관련) ---
	/** 캐릭터의 생명력입니다. 0이 되면 죽습니다. */
	Health UMETA(DisplayName = "Health"),
	/** 캐릭터의 스태미나입니다. 달리기, 구르기 등 특정 행동에 소모됩니다. */
	Stamina UMETA(DisplayName = "Stamina"),
	/** 캐릭터의 마나입니다. 마법이나 스킬 사용에 소모됩니다. */
	Mana UMETA(DisplayName = "Mana"),

	// --- Core Stats (핵심 능력치) ---
	/** 캐릭터의 힘입니다. 주로 물리 공격력에 영향을 줍니다. */
	Strength UMETA(DisplayName = "Strength"),
	/** 캐릭터의 민첩입니다. 공격 속도, 원거리 무기 정확도 등에 영향을 줍니다. */
	Dexterity UMETA(DisplayName = "Dexterity"),
	/** 캐릭터의 지능입니다. 주로 마법 공격력에 영향을 줍니다. */
	Intelligence UMETA(DisplayName = "Intelligence"),
	/** 캐릭터의 인내력입니다. 최대 체력, 방어력 등에 영향을 줍니다. */
	Fortitude UMETA(DisplayName = "Fortitude"),

	// --- Combat (전투 관련) ---
	/** 캐릭터의 기본 공격력입니다. 최종 피해량 계산에 사용됩니다. */
	AttackPower UMETA(DisplayName = "Attack Power"),
	/** 캐릭터의 방어력입니다. 받는 피해량을 감소시킵니다. */
	Defense UMETA(DisplayName = "Defense"),
	/** 치명타 공격이 발생할 확률입니다. */
	CriticalChance UMETA(DisplayName = "Critical Chance"),
	/** 적의 공격을 받았을 때 경직에 저항하는 능력입니다. (강인도) */
	Poise UMETA(DisplayName = "Poise"),

	// --- Movement (이동 관련) ---
	/** 걷는 속도의 배율입니다. 1.0이 기본값입니다. */
	WalkSpeedRate UMETA(DisplayName = "Walk Speed Rate"),
	/** 달리기 속도의 배율입니다. 1.0이 기본값입니다. */
	SprintSpeedRate UMETA(DisplayName = "Sprint Speed Rate"),
	/** 점프 높이의 배율입니다. 1.0이 기본값입니다. */
	JumpForceRate UMETA(DisplayName = "Jump Force Rate"),
	
	// --- Tactical (전술 관련) ---
	/** 무기 재장전 속도의 배율입니다. 1.0이 기본값이며, 값이 클수록 빨라집니다. */
	ReloadSpeedRate UMETA(DisplayName = "Reload Speed Rate"),
	/** 총기 반동 제어력의 배율입니다. 1.0이 기본값이며, 값이 클수록 반동이 줄어듭니다. */
	RecoilControlRate UMETA(DisplayName = "Recoil Control Rate"),

	// --- Regeneration (재생 관련) ---
	/** 초당 스태미나 재생량입니다. */
	StaminaRegen UMETA(DisplayName = "Stamina Regen"),
};

/**
 * @enum EModifierType
 * @brief 스탯 모디파이어의 연산 방식을 정의합니다.
 */
UENUM(BlueprintType)
enum class EModifierType : uint8
{
	/** 합연산: 스탯의 최종 값에 수치를 더합니다. (예: 힘 +10) */
	Add UMETA(DisplayName = "Additive"),
	/** 곱연산: 스탯의 최종 값에 수치를 곱합니다. (예: 공격력 20% 증가 -> 1.2 곱하기) */
	Multiply UMETA(DisplayName = "Multiplicative"),
};

/**
 * @struct FStatModifier
 * @brief 스탯을 변화시키는 최소 단위 구조체입니다. 버프, 디버프, 장비 효과 등을 표현하는 데 사용됩니다.
 */
USTRUCT(BlueprintType)
struct FStatModifier
{
	GENERATED_BODY()

	/** 모디파이어의 값입니다. 합연산일 경우 더해지는 수치, 곱연산일 경우 곱해지는 배율이 됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	float Value;

	/** 모디파이어의 연산 타입 (합연산 또는 곱연산) 입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	EModifierType Type;

	/**
	 * @brief 이 모디파이어를 제공한 주체(Source)에 대한 참조입니다.
	 * 주로 장비 아이템, 스킬, 버프 효과 등이 될 수 있으며, 해당 효과가 사라질 때 관련 모디파이어들을 한번에 제거하기 위해 사용됩니다.
	 * (예: '용기의 검' 장착 해제 시, '용기의 검'이 부여했던 모든 스탯 모디파이어를 이 Source를 통해 찾아 제거)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	TWeakObjectPtr<UObject> Source;

	FStatModifier() : Value(0.f), Type(EModifierType::Add), Source(nullptr) {}
	FStatModifier(float InValue, EModifierType InType, UObject* InSource)
		: Value(InValue), Type(InType), Source(InSource) {}
};

/**
 * @struct FModifiableAttribute
 * @brief 개별 스탯의 데이터를 관리하는 컨테이너입니다. (예: Health 스탯 하나에 대한 모든 정보)
 * 기본 값, 현재 적용된 모디파이어 목록, 그리고 최종 계산된 값을 포함합니다.
 */
USTRUCT(BlueprintType)
struct FModifiableAttribute
{
	GENERATED_BODY()

private:
	/** @brief 스탯의 순수 기본 값입니다. 어떤 모디파이어도 적용되지 않은 초기 값입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = "true"))
	float BaseValue;

	/**
	 * @brief 모든 모디파이어가 적용된 후의 최종 값입니다.
	 * 계산 비용을 줄이기 위해 캐싱되며, GetAttributeValue() 호출 시 이 값을 반환합니다.
	 * 모디파이어가 추가/제거될 때마다 재계산됩니다.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = "true"))
	float CurrentValue;

public:
	/** @brief 이 스탯에 현재 적용 중인 모든 모디파이어의 목록입니다. */
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = "true"))
	TArray<FStatModifier> Modifiers;

public:
	FModifiableAttribute() : BaseValue(0.f), CurrentValue(0.f) {}
	explicit FModifiableAttribute(float InBaseValue) : BaseValue(InBaseValue), CurrentValue(InBaseValue) {}

	float GetBaseValue() const { return BaseValue; }
	float GetCurrentValue() const { return CurrentValue; }
	void SetBaseValue(float NewBaseValue) { BaseValue = NewBaseValue; }
	void SetCurrentValue(float NewCurrentValue) { CurrentValue = NewCurrentValue; }
};

/**
 * @struct FAttributeData
 * @brief 데이터 테이블(UDataTable)을 사용하여 캐릭터의 초기 스탯을 설정하기 위한 구조체입니다.
 * 데이터 테이블의 각 행(Row)은 이 구조체에 매핑됩니다.
 */
USTRUCT(BlueprintType)
struct FAttributeData : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** @brief 초기화할 스탯의 종류입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	EAttributeType AttributeType;

	/** @brief 해당 스탯에 설정될 초기 기본 값(BaseValue)입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float BaseValue;

	FAttributeData() : AttributeType(EAttributeType::Health), BaseValue(0.f) {}
};
