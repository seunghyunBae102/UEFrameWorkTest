// GHOST IN THE MONITOR - Magazine Logistics Component
#include "Outer/BPC_MagLogistic.h"

UBP_MagLogistic::UBP_MagLogistic()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.TickInterval = 0.0f;
}

void UBP_MagLogistic::BeginPlay()
{
	Super::BeginPlay();
	
	// 초기 스프링 장력 계산
	RecalculateSpringTension();
}

bool UBP_MagLogistic::InsertBullet(UDA_AmmoType* Ammo, float& OutInsertTime)
{
	if (!Ammo)
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_MagLogistic::InsertBullet - Invalid ammo data asset!"));
		OutInsertTime = 0.0f;
		return false;
	}

	if (IsFull())
	{
		UE_LOG(LogTemp, Log, TEXT("BPC_MagLogistic::InsertBullet - Magazine is full! Capacity: %d"), Capacity);
		OutInsertTime = 0.0f;
		return false;
	}

	int32 BulletCountBefore = LoadedAmmo.Num();
	OutInsertTime = CalculateInsertTime(BulletCountBefore);

	// 탄을 배열의 끝에 추가 (스택 구조)
	LoadedAmmo.Add(Ammo);

	// 스프링 장력 재계산
	RecalculateSpringTension();

	// 이벤트 브로드캐스트
	OnBulletInserted.Broadcast(Ammo, LoadedAmmo.Num());

	UE_LOG(LogTemp, Log, TEXT("BPC_MagLogistic::InsertBullet - Inserted [%s] (%.2f sec). Total: %d/%d"),
		*Ammo->AmmoID.ToString(), OutInsertTime, LoadedAmmo.Num(), Capacity);

	// 탄창이 가득 찼을 경우
	if (IsFull())
	{
		OnMagazineFull.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("BPC_MagLogistic::InsertBullet - Magazine is now FULL!"));
	}

	return true;
}

bool UBP_MagLogistic::RemoveBullet(UDA_AmmoType*& OutRemovedAmmo)
{
	if (IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("BPC_MagLogistic::RemoveBullet - Magazine is empty!"));
		OutRemovedAmmo = nullptr;
		return false;
	}

	int32 LastIndex = LoadedAmmo.Num() - 1;
	OutRemovedAmmo = LoadedAmmo[LastIndex];
	LoadedAmmo.RemoveAt(LastIndex);

	// 스프링 장력 재계산
	RecalculateSpringTension();

	// 이벤트 브로드캐스트
	OnBulletRemoved.Broadcast(OutRemovedAmmo, LoadedAmmo.Num());

	UE_LOG(LogTemp, Log, TEXT("BPC_MagLogistic::RemoveBullet - Removed [%s]. Total: %d/%d"),
		*OutRemovedAmmo->AmmoID.ToString(), LoadedAmmo.Num(), Capacity);

	// 탄창이 비었을 경우
	if (IsEmpty())
	{
		OnMagazineEmpty.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("BPC_MagLogistic::RemoveBullet - Magazine is now EMPTY!"));
	}

	return true;
}

EMagazineWeightStatus UBP_MagLogistic::EstimateWeightStatus() const
{
	int32 BulletCount = LoadedAmmo.Num();

	if (BulletCount == 0)
		return EMagazineWeightStatus::Empty;
	else if (BulletCount <= 3)
		return EMagazineWeightStatus::Light;
	else if (BulletCount <= 7)
		return EMagazineWeightStatus::Medium;
	else if (BulletCount < Capacity)
		return EMagazineWeightStatus::Heavy;
	else
		return EMagazineWeightStatus::Full;
}

float UBP_MagLogistic::GetCurrentWeight() const
{
	float TotalWeight = 0.0f;

	for (const UDA_AmmoType* Ammo : LoadedAmmo)
	{
		if (Ammo)
		{
			TotalWeight += Ammo->WeightPerRound;
		}
	}

	return TotalWeight;
}

void UBP_MagLogistic::ClearMagazine()
{
	if (!IsEmpty())
	{
		LoadedAmmo.Empty();
		SpringTension = 0.0f;
		OnMagazineEmpty.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("BPC_MagLogistic::ClearMagazine - Magazine cleared!"));
	}
}

void UBP_MagLogistic::RecalculateSpringTension()
{
	// 스프링 장력 = 현재 탄의 개수 / 최대 용량
	// 0.0 ~ 1.0 사이의 값
	SpringTension = static_cast<float>(LoadedAmmo.Num()) / static_cast<float>(Capacity);
}

float UBP_MagLogistic::CalculateInsertTime(int32 BulletCountBefore) const
{
	// 기본 삽탄 시간에 스프링 장력 보정을 곱함
	// 탄이 적을수록 빠르고, 많을수록 느림
	float SpringTension_Before = static_cast<float>(BulletCountBefore) / static_cast<float>(Capacity);
	
	// 스프링 장력이 높을수록 더 오래 걸림 (1.0 ~ 2.0배)
	float TensionMultiplier = 1.0f + SpringTension_Before;
	
	return BaseInsertTimePerBullet * TensionMultiplier;
}
