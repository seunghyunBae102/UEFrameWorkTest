// GHOST IN THE MONITOR - Inner World RPG Stats Component

#include "Inner/BPC_RPGStats.h"
#include "Data/DA_CharacterBaseStats.h"
#include "TimerManager.h"

UBPC_RPGStats::UBPC_RPGStats()
{
	// Tick을 사용하지 않도록 설정합니다. (BasicRule.md 준수)
	PrimaryComponentTick.bCanEverTick = false;

	CurrentPoise = 0.0f;
	MaxPoise = 0.0f;
}

void UBPC_RPGStats::BeginPlay()
{
	Super::BeginPlay();

	// 데이터 에셋이 유효한지 확인하고, 스탯을 초기화합니다.
	if (ensure(BaseStatsDataAsset))
	{
		MaxPoise = BaseStatsDataAsset->MaxPoise;
		CurrentPoise = MaxPoise;

		// 초기 상태를 UI 등에 알리기 위해 이벤트를 호출합니다.
		OnPoiseChanged.Broadcast(CurrentPoise, MaxPoise);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BPC_RPGStats on %s is missing BaseStatsDataAsset!"), *GetOwner()->GetName());
	}
}

void UBPC_RPGStats::ApplyPoiseDamage(float DamageAmount)
{
	if (DamageAmount <= 0.0f || CurrentPoise <= 0.0f)
	{
		return;
	}

	CurrentPoise = FMath::Max(0.0f, CurrentPoise - DamageAmount);
	UE_LOG(LogTemp, Log, TEXT("%s took %f poise damage. Current Poise: %f / %f"), *GetOwner()->GetName(), DamageAmount, CurrentPoise, MaxPoise);

	OnPoiseChanged.Broadcast(CurrentPoise, MaxPoise);

	// 강인도가 0이 되면 '파괴' 이벤트를 호출합니다.
	if (CurrentPoise <= 0.0f)
	{
		OnPoiseBroken.Broadcast();
		UE_LOG(LogTemp, Warning, TEXT("%s's poise has been broken!"), *GetOwner()->GetName());
	}

	// 기존의 회복 타이머들을 모두 중지하고, 새로운 회복 딜레이 타이머를 시작합니다.
	GetWorld()->GetTimerManager().ClearTimer(PoiseRecoveryDelayTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(PoiseRecoveryTickTimerHandle);

	if (BaseStatsDataAsset)
	{
		GetWorld()->GetTimerManager().SetTimer(
			PoiseRecoveryDelayTimerHandle,
			this,
			&UBPC_RPGStats::StartPoiseRecovery,
			BaseStatsDataAsset->PoiseRecoveryDelay,
			false
		);
	}
}

void UBPC_RPGStats::StartPoiseRecovery()
{
	// 0.1초마다 RecoverPoiseTick 함수를 반복 호출하는 타이머를 시작합니다.
	GetWorld()->GetTimerManager().SetTimer(
		PoiseRecoveryTickTimerHandle,
		this,
		&UBPC_RPGStats::RecoverPoiseTick,
		0.1f,
		true
	);
}

void UBPC_RPGStats::RecoverPoiseTick()
{
	const float RecoveryAmount = (BaseStatsDataAsset->PoiseRecoveryRate * 0.1f); // 0.1초당 회복량
	CurrentPoise = FMath::Min(MaxPoise, CurrentPoise + RecoveryAmount);

	OnPoiseChanged.Broadcast(CurrentPoise, MaxPoise);

	// 강인도가 최대로 회복되면 타이머를 중지합니다.
	if (CurrentPoise >= MaxPoise)
	{
		GetWorld()->GetTimerManager().ClearTimer(PoiseRecoveryTickTimerHandle);
	}
}