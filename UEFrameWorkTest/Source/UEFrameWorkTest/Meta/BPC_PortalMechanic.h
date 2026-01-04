// GHOST IN THE MONITOR - Portal Mechanic Component (Meta Bridge)
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BPC_PortalMechanic.generated.h"

/**
 * BPC_PortalMechanic - Monitor Hit과 Inner World 연결
 * 
 * 역할:
 * - Outer World의 Monitor 총기 발사를 Inner World로 변환
 * - UV 좌표를 Inner World 벡터로 매핑
 * - 충격력을 받은 모든 Inner World 객체에 물리적 영향 적용
 * 
 * 규칙:
 * - Tick 비활성화 (함수 호출로만 작동)
 * - Cast 금지 (BPI_RPGCombat Interface로만 통신)
 * - 영향받을 수 있는 모든 액터에 Overlap 기반 힘 적용
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UEFRAMEWORKTEST_API UBP_PortalMechanic : public UActorComponent
{
	GENERATED_BODY()

public:
	UBP_PortalMechanic();

	virtual void BeginPlay() override;

	// ==================== Properties ====================

	// Inner World의 맵 크기 (미터 단위)
	// Monitor의 2D 픽셀 좌표를 이 크기에 맞게 스케일링
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FVector2D InnerWorldMapSize = FVector2D(10000.0f, 10000.0f);

	// Inner World의 기준점 (월드 좌표)
	// UV (0, 0)이 이 좌표로 매핑됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FVector InnerWorldOrigin = FVector(0.0f, 0.0f, 0.0f);

	// Monitor 해상도 (픽셀)
	// UV 계산에 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FVector2D MonitorResolution = FVector2D(1920.0f, 1080.0f);

	// 충격이 영향을 미칠 최대 거리 (Inner World에서의 반경)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	float RadialForceRadius = 500.0f;

	// ==================== Events ====================

	// Monitor Hit가 Inner World에 영향을 미쳤을 때
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMetaImpactApplied, FVector, ImpactPoint, float, Force, float, Radius);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMetaImpactApplied OnMetaImpactApplied;

	// ==================== Methods ====================

	/**
	 * Outer World의 Monitor Hit를 처리하여 Inner World에 영향 적용
	 * @param OuterHitResult: Outer World의 충돌 정보
	 * @param ImpactForce: 물리적 힘의 크기
	 * 
	 * 로직:
	 * 1. HitResult에서 UV 좌표 추출 (UGameplayStatics::FindCollisionUV)
	 * 2. UV를 Inner World 벡터로 변환
	 * 3. 해당 위치 주변의 모든 BPI_RPGCombat 구현 액터 찾기
	 * 4. ReceiveMetaImpact() 호출로 힘 적용
	 */
	UFUNCTION(BlueprintCallable, Category = "Portal")
	void ProcessMonitorHit(const FHitResult& OuterHitResult, float ImpactForce);

	/**
	 * UV 좌표를 Inner World 벡터로 변환
	 * @param UV: (0,0) ~ (1,1) 범위의 2D 좌표
	 * @return Inner World의 3D 위치
	 */
	UFUNCTION(BlueprintPure, Category = "Portal")
	FVector ConvertUVToInnerWorldLocation(FVector2D UV) const;

	/**
	 * 픽셀 좌표를 UV로 변환
	 * @param PixelX: 모니터 픽셀 X
	 * @param PixelY: 모니터 픽셀 Y
	 * @return (0,0) ~ (1,1) 범위의 UV
	 */
	UFUNCTION(BlueprintPure, Category = "Portal")
	FVector2D ConvertPixelToUV(float PixelX, float PixelY) const;

	/**
	 * Inner World의 특정 위치 주변에 모든 영향받을 수 있는 액터 찾기
	 * @param CenterPoint: Inner World 중심점
	 * @param SearchRadius: 검색 반경
	 * @return BPI_RPGCombat을 구현한 액터 배열
	 */
	UFUNCTION(BlueprintCallable, Category = "Portal")
	void FindAndApplyImpactToActors(FVector CenterPoint, float ImpactForce, float SearchRadius);

private:
	/**
	 * 단일 액터에 Meta Impact 적용
	 */
	void ApplyMetaImpactToActor(AActor* TargetActor, FVector ImpactPoint, float Force, float RadialRadius);
};
