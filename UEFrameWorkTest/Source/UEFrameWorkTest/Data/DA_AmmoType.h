// GHOST IN THE MONITOR - Outer World Ammunition Data Asset
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_AmmoType.generated.h"

/**
 * 탄약 규격 - Outer World
 * 불량탄, 관통력 등 현실적 탄약 시뮬레이션을 담당
 */
UCLASS(BlueprintType, Const)
class UEFRAMEWORKTEST_API UDA_AmmoType : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 구경 이름 (예: "9x19mm Parabellum")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	FName CaliberName = TEXT("9x19mm");

	// 불량탄 확률 (0.0 ~ 1.0). 높으면 Jamming 발생.
	// 예: 0.05 = 5% 확률로 불발
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reliability", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MisfireChance = 0.02f;

	// 대인 저지력 (Inner 몬스터에게 들어가는 실제 데미지)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float FleshDamage = 15.0f;

	// 관통력 (모니터 유리 등 사물 관통 여부)
	// 0.0 ~ 1.0: 높을수록 더 많은 물질을 관통
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ballistics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PenetrationPower = 0.3f;

	// 탄약의 무게 (탄창 무게 계산에 사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physical")
	float WeightPerRound = 0.25f;

	// 탄약의 시각적/음성 프리셋 (로그, 사운드 구분용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	FName AmmoID = TEXT("556NATO");
};
