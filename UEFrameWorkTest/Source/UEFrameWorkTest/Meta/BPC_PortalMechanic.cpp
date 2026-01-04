// GHOST IN THE MONITOR - Portal Mechanic Component (Meta Bridge)
#include "Meta/BPC_PortalMechanic.h"
#include "Interface/BPI_RPGCombat.h"
#include "Kismet/GameplayStatics.h"

UBP_PortalMechanic::UBP_PortalMechanic()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.TickInterval = 0.0f;
}

void UBP_PortalMechanic::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("BPC_PortalMechanic::BeginPlay - Portal mechanic initialized!"));
	UE_LOG(LogTemp, Log, TEXT("  Inner World Map Size: %.0f x %.0f"), InnerWorldMapSize.X, InnerWorldMapSize.Y);
	UE_LOG(LogTemp, Log, TEXT("  Monitor Resolution: %.0f x %.0f"), MonitorResolution.X, MonitorResolution.Y);
}

void UBP_PortalMechanic::ProcessMonitorHit(const FHitResult& OuterHitResult, float ImpactForce)
{
	if (!OuterHitResult.GetComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_PortalMechanic::ProcessMonitorHit - Invalid hit result!"));
		return;
	}

	// 1. HitResult에서 UV 좌표 추출
	FVector2D UV = FVector2D::ZeroVector;
	if (OuterHitResult.Component.IsValid())
	{
		// UGameplayStatics::FindCollisionUV를 사용하여 충돌 지점의 UV 추출
		UGameplayStatics::FindCollisionUV(OuterHitResult, 0, UV);
	}

	UE_LOG(LogTemp, Log, TEXT("BPC_PortalMechanic::ProcessMonitorHit - UV: (%.3f, %.3f)"), UV.X, UV.Y);

	// 2. UV를 Inner World 벡터로 변환
	FVector InnerWorldImpactPoint = ConvertUVToInnerWorldLocation(UV);

	UE_LOG(LogTemp, Log, TEXT("BPC_PortalMechanic::ProcessMonitorHit - Impact at Inner World: (%.2f, %.2f, %.2f), Force: %.2f"),
		InnerWorldImpactPoint.X, InnerWorldImpactPoint.Y, InnerWorldImpactPoint.Z, ImpactForce);

	// 3. 해당 위치의 모든 영향받을 수 있는 액터 찾기 및 힘 적용
	FindAndApplyImpactToActors(InnerWorldImpactPoint, ImpactForce, RadialForceRadius);

	// 4. 이벤트 브로드캐스트
	OnMetaImpactApplied.Broadcast(InnerWorldImpactPoint, ImpactForce, RadialForceRadius);
}

FVector UBP_PortalMechanic::ConvertUVToInnerWorldLocation(FVector2D UV) const
{
	// UV는 (0,0) ~ (1,1) 범위
	// 이를 Inner World의 맵 크기에 스케일링
	FVector WorldLocation = InnerWorldOrigin;

	WorldLocation.X += UV.X * InnerWorldMapSize.X;
	WorldLocation.Y += UV.Y * InnerWorldMapSize.Y;
	// Z는 변경하지 않음 (동일 평면)

	return WorldLocation;
}

FVector2D UBP_PortalMechanic::ConvertPixelToUV(float PixelX, float PixelY) const
{
	// 픽셀 좌표를 0~1 범위의 UV로 정규화
	FVector2D UV;
	UV.X = FMath::Clamp(PixelX / MonitorResolution.X, 0.0f, 1.0f);
	UV.Y = FMath::Clamp(PixelY / MonitorResolution.Y, 0.0f, 1.0f);

	return UV;
}

void UBP_PortalMechanic::FindAndApplyImpactToActors(FVector CenterPoint, float ImpactForce, float SearchRadius)
{
	if (!GetWorld())
	{
		return;
	}

	// 구 형태의 범위 검색으로 영향받을 모든 액터 찾기
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(SearchRadius);
	TArray<FOverlapResult> OverlapResults;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		CenterPoint,
		FQuat::Identity,
		FCC_WorldStatic | FCC_WorldDynamic | FCC_Pawn,
		CollisionShape,
		QueryParams
	);

	if (bHit)
	{
		UE_LOG(LogTemp, Log, TEXT("BPC_PortalMechanic::FindAndApplyImpactToActors - Found %d actors in impact radius"), OverlapResults.Num());

		for (FOverlapResult& Overlap : OverlapResults)
		{
			AActor* TargetActor = Overlap.GetActor();
			if (TargetActor)
			{
				ApplyMetaImpactToActor(TargetActor, CenterPoint, ImpactForce, SearchRadius);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("BPC_PortalMechanic::FindAndApplyImpactToActors - No actors found in impact radius"));
	}
}

void UBP_PortalMechanic::ApplyMetaImpactToActor(AActor* TargetActor, FVector ImpactPoint, float Force, float RadialRadius)
{
	if (!TargetActor)
	{
		return;
	}

	// BPI_RPGCombat 인터페이스 구현 여부 확인
	if (TargetActor->Implements<UBP_RPGCombat>())
	{
		// 거리에 따른 힘 감쇠
		float Distance = FVector::Dist(TargetActor->GetActorLocation(), ImpactPoint);
		float AttenuatedForce = Force * FMath::Max(0.0f, (RadialRadius - Distance) / RadialRadius);

		UE_LOG(LogTemp, Log, TEXT("BPC_PortalMechanic::ApplyMetaImpactToActor - Applying impact to [%s]. Distance: %.2f, Attenuated Force: %.2f"),
			*TargetActor->GetName(), Distance, AttenuatedForce);

		// ReceiveMetaImpact 호출
		IBP_RPGCombat::Execute_ReceiveMetaImpact(TargetActor, ImpactPoint, AttenuatedForce, RadialRadius);
	}
	else
	{
		// 물리 기반 객체 (Simple Physics)에 직접 충격 적용
		if (TargetActor->GetRootComponent() && TargetActor->GetRootComponent()->IsSimulatingPhysics())
		{
			FVector Direction = (TargetActor->GetActorLocation() - ImpactPoint).GetSafeNormal();
			if (Direction.IsZero())
			{
				Direction = FVector::UpVector;
			}

			float Distance = FVector::Dist(TargetActor->GetActorLocation(), ImpactPoint);
			float AttenuatedForce = Force * FMath::Max(0.0f, (RadialRadius - Distance) / RadialRadius);

			TargetActor->GetRootComponent()->AddImpulse(Direction * AttenuatedForce * 1000.0f, NAME_None, true);

			UE_LOG(LogTemp, Log, TEXT("BPC_PortalMechanic::ApplyMetaImpactToActor - Applied physics impulse to [%s]"), *TargetActor->GetName());
		}
	}
}
