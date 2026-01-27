// Copyright 2024, Korstian Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Attributes/AttributeSystemTypes.h"
#include "ParkourComponent.generated.h"

// Forward declarations
class ACharacter;
class UCharacterMovementComponent;
class UAttributeComponent;
class UCharacterStateComponent;
class UMotionWarpingComponent;
class UAnimMontage;

/**
 * @struct FLedgeDetectionResult
 * @brief 렛지(모서리) 탐지 트레이스의 결과를 담는 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FLedgeDetectionResult
{
	GENERATED_BODY()

	/** @brief 렛지 발견 여부입니다. */
	bool bLedgeFound = false;
	/** @brief 발견된 렛지의 월드 위치입니다. 캐릭터가 올라설 목표 지점이 됩니다. */
	FVector LedgeLocation;
	/** @brief 발견된 렛지(벽)의 노멀 벡터입니다. 캐릭터가 바라볼 방향을 결정하는 데 사용됩니다. */
	FVector LedgeNormal;
};


/**
 * @class UParkourComponent
 * @brief 캐릭터의 파쿠르(벽 타기, 장애물 넘기 등) 액션을 담당하는 컴포넌트입니다.
 * 실시간 환경 트레이스를 통해 파쿠르 가능 지점을 감지하고, Motion Warping을 사용하여
 * 애니메이션이 지형에 정확히 맞춰지도록 동적으로 보정합니다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEFRAMEWORKTEST_API UParkourComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UParkourComponent();

protected:
	virtual void BeginPlay() override;

private:
	// --- Cached component references (캐시된 컴포넌트 참조) ---
	UPROPERTY() TObjectPtr<ACharacter> OwnerCharacter;
	UPROPERTY() TObjectPtr<UCharacterMovementComponent> CMC;
	UPROPERTY() TObjectPtr<UAttributeComponent> AttributeComp;
	UPROPERTY() TObjectPtr<UCharacterStateComponent> StateComp;
	/** @brief 애니메이션의 루트 모션을 목표 지점에 동적으로 보정하는 데 사용되는 핵심 컴포넌트입니다. */
	UPROPERTY() TObjectPtr<UMotionWarpingComponent> MotionWarpingComp;

public:
	/**
	 * @brief 파쿠르 액션을 시작하는 메인 진입점 함수입니다. 주로 캐릭터의 점프 입력에 연결됩니다.
	 * 주변을 탐지하여 수행 가능한 파쿠르 액션(맨틀, 볼트 등)을 찾아 실행합니다.
	 * @return 파쿠르 액션을 시작했다면 true, 그렇지 않다면 false를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Parkour")
	bool TryParkour();

protected:
	// --- DETECTION (탐지) ---
	
	// Vault(넘기) 탐지 관련 파라미터
	UPROPERTY(EditDefaultsOnly, Category = "Parkour|Detection")
	float VaultTraceDistance = 150.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour|Detection")
	float MaxVaultHeight = 100.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour|Detection")
	float MinVaultHeight = 30.f;

	// Ledge(매달리기) 탐지 관련 파라미터
	UPROPERTY(EditDefaultsOnly, Category = "Parkour|Detection")
	float LedgeTraceDistance = 150.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour|Detection")
	float MaxLedgeHeight = 250.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour|Detection")
	float MinLedgeHeight = 120.f;

	/** @brief 캐릭터 전방에 넘을 수 있는(Vaultable) 장애물이 있는지 탐지합니다. */
	bool DetectVaultableObject(FHitResult& OutHit);
	/** @brief 캐릭터 전상방에 매달릴 수 있는(Mantleable) 렛지가 있는지 탐지합니다. */
	FLedgeDetectionResult DetectLedge();

	// --- ACTIONS (액션) ---

	/** @brief 탐지된 장애물 정보를 바탕으로 Vault 액션을 수행합니다. 관련 몽타주를 재생하고 Motion Warping 타겟을 설정합니다. */
	void PerformVault(const FHitResult& ObstacleHit);
	/** @brief 탐지된 렛지 정보를 바탕으로 Mantle 액션을 수행합니다. 관련 몽타주를 재생하고 Motion Warping 타겟을 설정합니다. */
	void PerformMantle(const FLedgeDetectionResult& LedgeResult);

	// --- ANIMATION (애니메이션) ---
	UPROPERTY(EditDefaultsOnly, Category = "Parkour|Animation")
	TObjectPtr<UAnimMontage> VaultMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Parkour|Animation")
	TObjectPtr<UAnimMontage> MantleMontage;


	// --- STATS (스탯 연동) ---
	
	/** @brief AttributeComponent의 델리게이트에 바인딩되어, 스탯 변경 시 파쿠르 관련 성능(속도, 스태미나 소모 등)을 업데이트합니다. */
	UFUNCTION()
	void SyncParkourStats(EAttributeType Type, float NewValue, float Delta);
	
	float ParkourStaminaCost = 15.f;
	float ClimbSpeedRate = 1.f;

private:
	/** @brief 파쿠르 액션이 끝난 후 캐릭터의 상태를 원래대로 복원하는 헬퍼 함수입니다. */
	void FinishParkourAction();
};
