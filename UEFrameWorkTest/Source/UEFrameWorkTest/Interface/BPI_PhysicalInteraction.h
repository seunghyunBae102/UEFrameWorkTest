// GHOST IN THE MONITOR - Physical Interaction Interface
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BPI_PhysicalInteraction.generated.h"

/**
 * 손으로 할 수 있는 물리적 상호작용 타입
 */
UENUM(BlueprintType)
enum class EPhysicalAction : uint8
{
	Inspect = 0,       // 자세히 관찰 (탄창 무게 가늠, 총기 약실 확인)
	InsertBullet = 1,  // 탄알 하나 넣기
	PullBolt = 2,      // 노리쇠 당기기
	ClearJam = 3,      // 걸린 탄 제거
	ToggleSafety = 4,  // 안전장치 해제/걸기
	AimDown = 5        // 조준
};

/**
 * BPI_PhysicalInteraction - Outer World 물리적 상호작용 인터페이스
 * 
 * 역할:
 * - Outer World의 모든 물리적 상호작용을 정의
 * - 액터가 이 인터페이스를 구현하면, 손(플레이어)이 상호작용 가능
 * - Cast 없이 Interface Call로만 통신
 * 
 * 규칙:
 * - 반환값 없음: 모든 작업은 애니메이션과 Delegate를 통해 결과 전달
 * - 인터페이스는 로직을 포함하지 않음 (선언만)
 * - 구현은 각 Component에서 담당
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UBP_PhysicalInteraction : public UInterface
{
	GENERATED_BODY()
};

class IBP_PhysicalInteraction
{
	GENERATED_BODY()

public:
	/**
	 * 아이템을 자세히 관찰
	 * - 탄창: 무게를 통해 대략적 탄 개수 추정 (정확한 개수는 모름)
	 * - 총기: 약실에 탄이 있는지 확인
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PhysicalInteraction")
	void InspectItem();

	/**
	 * 아이템에 물리적 힘 가하기
	 * @param ActionType: 어떤 작업을 수행할 것인가
	 * - InsertBullet: 탄 넣기
	 * - PullBolt: 노리쇠 당기기
	 * - ClearJam: 잼 해결
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PhysicalInteraction")
	void ManipulateItem(EPhysicalAction ActionType);

	/**
	 * 아이템의 무게 피드백 (손으로 느끼는 무게)
	 * @return 상태 문자열 (Empty, Light, Heavy, Full 등)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PhysicalInteraction")
	FString GetWeightFeedback() const;
};
