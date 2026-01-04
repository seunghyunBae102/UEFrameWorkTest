// GHOST IN THE MONITOR - Outer World Weapon Part Data Asset
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_WeaponPart.generated.h"

/**
 * 무기 부품 타입 (Modding Part)
 */
UENUM(BlueprintType)
enum class EWeaponPartType : uint8
{
	Barrel = 0,      // 총열
	Receiver = 1,    // 수신기 (총 몸체)
	Stock = 2,       // 개머리판
	Magazine = 3,    // 탄창
	Muzzle = 4,      // 총구 (소음기 등)
	Trigger = 5,     // 트리거
	Other = 6
};

/**
 * 총기 부품 (Modding Part) - Outer World
 * 신뢰성, 인체공학, 반동 제어 등을 정의
 */
UCLASS(BlueprintType, Const)
class UEFRAMEWORKTEST_API UDA_WeaponPart : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 부품 타입
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	EWeaponPartType PartType = EWeaponPartType::Barrel;

	// 신뢰성 보정 (내구도 감소 속도 영향)
	// 1.0 = 중립, 1.2 = 20% 더 신뢰할 수 있음, 0.8 = 20% 덜 신뢰함
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reliability")
	float ReliabilityModifier = 1.0f;

	// 인체공학 보정 (ADS 속도, 소음 유발 정도 영향)
	// 1.0 = 중립, 1.2 = 20% 더 좋은 인체공학
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ergonomics")
	float ErgonomicsModifier = 1.0f;

	// 반동 제어 보정
	// 1.0 = 중립, 1.2 = 20% 더 적은 반동
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ballistics")
	float RecoilControl = 1.0f;

	// 부품의 무게 (총 무게 계산에 사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physical")
	float WeightGrams = 100.0f;

	// 부품의 설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Description")
	FText PartDescription;
};
