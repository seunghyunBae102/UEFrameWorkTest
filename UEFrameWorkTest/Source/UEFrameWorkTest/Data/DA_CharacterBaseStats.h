// GHOST IN THE MONITOR - Inner World Character Base Stats
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_CharacterBaseStats.generated.h"

/**
 * DA_CharacterBaseStats
 * 
 * Inner World 캐릭터의 기본 스탯을 정의하는 데이터 에셋입니다.
 * 강인도(Poise) 시스템의 핵심 수치를 포함하며, 데이터 주도 설계를 위해 사용됩니다.
 * 기획자는 이 에셋의 인스턴스를 생성하고 수정하여 캐릭터별 밸런스를 조정할 수 있습니다.
 */
UCLASS(BlueprintType)
class UEFRAMEWORKTEST_API UDA_CharacterBaseStats : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * @brief 최대 강인도 (Poise)
	 * 이 수치가 0이 되면 캐릭터는 '스태거(Stagger)' 상태에 빠집니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise Stats")
	float MaxPoise = 100.0f;

	/**
	 * @brief 강인도 회복 대기 시간 (초)
	 * 마지막으로 강인도 피해를 입은 후, 이 시간만큼 지나야 강인도 회복이 시작됩니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise Stats")
	float PoiseRecoveryDelay = 3.0f;

	/**
	 * @brief 초당 강인도 회복량
	 * 대기 시간이 지난 후, 초당 이 수치만큼 강인도가 회복됩니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise Stats")
	float PoiseRecoveryRate = 20.0f;
};