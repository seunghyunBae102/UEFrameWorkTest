// GHOST IN THE MONITOR - Inner World Equipment Data Asset
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_InnerEquipment.generated.h"

/**
 * RPG 4대 스탯 구조체
 */
USTRUCT(BlueprintType)
struct FInnerAttributes
{
	GENERATED_BODY()

	// 근력: 물리 데미지 보정, 소지 중량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Strength = 10.0f;

	// 기량: 공속, 낙법, 치명타 보정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Dexterity = 10.0f;

	// 지력: 아이템 효율, 약점 간파
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Intelligence = 10.0f;

	// 생명력: Max HP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Vitality = 10.0f;
};

/**
 * 무기 보정 등급 (Scaling)
 */
UENUM(BlueprintType)
enum class EStatScaling : uint8
{
	S = 5,   // S등급: 180%
	A = 4,   // A등급: 150%
	B = 3,   // B등급: 120%
	C = 2,   // C등급: 100%
	D = 1,   // D등급: 80%
	None = 0 // 보정 없음: 0%
};

/**
 * Inner Attribute 타입
 */
UENUM(BlueprintType)
enum class EInnerAttributeType : uint8
{
	Strength = 0,
	Dexterity = 1,
	Intelligence = 2,
	Vitality = 3,
	Max = 4
};

/**
 * 인게임 장비 (무기/방어구) - Inner World
 * 데이터만 관리하고, 로직은 BPC_RPGStats에서 수행
 */
UCLASS(BlueprintType, Const)
class UEFRAMEWORKTEST_API UDA_InnerEquipment : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 기본 물리 공격력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float BasePhysPower = 10.0f;

	// 기본 마법 공격력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float BaseMagicPower = 0.0f;

	// 강인도 감쇄력 (공격 시) / 강인도 제공량 (방어구)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	float PoiseValue = 0.0f;

	// 스탯 보정치 (Map: Attribute -> Scaling Grade)
	// 예: {Strength : A, Dexterity : D}
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
	TMap<EInnerAttributeType, EStatScaling> ScalingMap;

	// 장비 무게 (Inner 캐릭터의 구르기 속도에 영향)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physical")
	float Weight = 1.0f;

	/**
	 * Scaling Grade를 실수로 변환 (곱셈 보정치)
	 * @param Scaling: S=1.8, A=1.5, B=1.2, C=1.0, D=0.8, None=0.0
	 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	static float ScalingToMultiplier(EStatScaling Scaling);
};
