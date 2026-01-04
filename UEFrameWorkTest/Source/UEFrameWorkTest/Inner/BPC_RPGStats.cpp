// GHOST IN THE MONITOR - Inner World RPG Stats Component
#include "Inner/BPC_RPGStats.h"
#include "TimerManager.h"
#include "Engine/World.h"

UBP_RPGStats::UBP_RPGStats()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.TickInterval = 0.0f;
}

void UBP_RPGStats::BeginPlay()
{
	Super::BeginPlay();

	// 초기 강인도 설정
	CurrentPoise = MaxPoise;

	UE_LOG(LogTemp, Log, TEXT("BPC_RPGStats::BeginPlay - Stats initialized. Base Poise: %.2f / %.2f"),
		CurrentPoise, MaxPoise);
}

void UBP_RPGStats::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 강인도 회복 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PoiseRecoveryTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UBP_RPGStats::EquipGear(UDA_InnerEquipment* Gear)
{
	if (!Gear)
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_RPGStats::EquipGear - Invalid gear!"));
		return;
	}

	if (EquippedGear.Contains(Gear))
	{
		UE_LOG(LogTemp, Log, TEXT("BPC_RPGStats::EquipGear - Gear already equipped!"));
		return;
	}

	EquippedGear.Add(Gear);
	MaxPoise += Gear->PoiseValue;

	BroadcastStatsChange();

	UE_LOG(LogTemp, Log, TEXT("BPC_RPGStats::EquipGear - Equipped gear. New Max Poise: %.2f"), MaxPoise);
}

void UBP_RPGStats::UnequipGear(UDA_InnerEquipment* Gear)
{
	if (!Gear)
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_RPGStats::UnequipGear - Invalid gear!"));
		return;
	}

	if (EquippedGear.Remove(Gear) > 0)
	{
		MaxPoise -= Gear->PoiseValue;
		MaxPoise = FMath::Max(1.0f, MaxPoise);

		BroadcastStatsChange();

		UE_LOG(LogTemp, Log, TEXT("BPC_RPGStats::UnequipGear - Unequipped gear. New Max Poise: %.2f"), MaxPoise);
	}
}

float UBP_RPGStats::CalculateAttackPower(UDA_InnerEquipment* Weapon)
{
	if (!Weapon)
	{
		return 0.0f;
	}

	float FinalDamage = Weapon->BasePhysPower;

	// 근력(Strength) 보정 적용
	float StrengthScaling = CalculateTotalScaling(EInnerAttributeType::Strength);
	FinalDamage *= StrengthScaling;

	UE_LOG(LogTemp, Log, TEXT("BPC_RPGStats::CalculateAttackPower - Base: %.2f, Strength Scaling: %.2f, Final: %.2f"),
		Weapon->BasePhysPower, StrengthScaling, FinalDamage);

	return FinalDamage;
}

float UBP_RPGStats::CalculateMagicPower(UDA_InnerEquipment* Weapon)
{
	if (!Weapon)
	{
		return 0.0f;
	}

	float FinalDamage = Weapon->BaseMagicPower;

	// 지력(Intelligence) 보정 적용
	float IntelligenceScaling = CalculateTotalScaling(EInnerAttributeType::Intelligence);
	FinalDamage *= IntelligenceScaling;

	UE_LOG(LogTemp, Log, TEXT("BPC_RPGStats::CalculateMagicPower - Base: %.2f, Intelligence Scaling: %.2f, Final: %.2f"),
		Weapon->BaseMagicPower, IntelligenceScaling, FinalDamage);

	return FinalDamage;
}

void UBP_RPGStats::TakePoiseDamage(float PoiseDamage)
{
	if (PoiseDamage <= 0.0f)
	{
		return;
	}

	CurrentPoise -= PoiseDamage;
	CurrentPoise = FMath::Max(0.0f, CurrentPoise);

	UE_LOG(LogTemp, Log, TEXT("BPC_RPGStats::TakePoiseDamage - Took %.2f poise damage. Current: %.2f / %.2f"),
		PoiseDamage, CurrentPoise, MaxPoise);

	OnPoiseChanged.Broadcast(CurrentPoise, MaxPoise);

	// 강인도가 0이 되면 Stagger 상태 진입
	if (CurrentPoise <= 0.0f && !HasStateTag(FGameplayTag::RequestGameplayTag(FName("State.Stagger"))))
	{
		AddStateTag(FGameplayTag::RequestGameplayTag(FName("State.Stagger")));
		OnStaggerState.Broadcast();
		UE_LOG(LogTemp, Warning, TEXT("BPC_RPGStats::TakePoiseDamage - STAGGERED!"));
	}

	// 강인도 회복 타이머 재설정
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PoiseRecoveryTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			PoiseRecoveryTimerHandle,
			this,
			&UBP_RPGStats::TickPoiseRecovery,
			1.0f,
			true,
			PoiseRecoveryDelay
		);
	}
}

void UBP_RPGStats::RecoverPoise(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	CurrentPoise += Amount;
	CurrentPoise = FMath::Min(CurrentPoise, MaxPoise);

	UE_LOG(LogTemp, Log, TEXT("BPC_RPGStats::RecoverPoise - Recovered %.2f poise. Current: %.2f / %.2f"),
		Amount, CurrentPoise, MaxPoise);

	OnPoiseChanged.Broadcast(CurrentPoise, MaxPoise);
}

void UBP_RPGStats::ExitStagger()
{
	RemoveStateTag(FGameplayTag::RequestGameplayTag(FName("State.Stagger")));
	UE_LOG(LogTemp, Log, TEXT("BPC_RPGStats::ExitStagger - Exited stagger state!"));
}

void UBP_RPGStats::AddStateTag(const FGameplayTag& Tag)
{
	StateTagContainer.AddTag(Tag);
	UE_LOG(LogTemp, Log, TEXT("BPC_RPGStats::AddStateTag - Added tag: %s"), *Tag.ToString());
}

void UBP_RPGStats::RemoveStateTag(const FGameplayTag& Tag)
{
	StateTagContainer.RemoveTag(Tag);
	UE_LOG(LogTemp, Log, TEXT("BPC_RPGStats::RemoveStateTag - Removed tag: %s"), *Tag.ToString());
}

bool UBP_RPGStats::HasStateTag(const FGameplayTag& Tag) const
{
	return StateTagContainer.HasTag(Tag);
}

bool UBP_RPGStats::IsStaggering() const
{
	return HasStateTag(FGameplayTag::RequestGameplayTag(FName("State.Stagger")));
}

void UBP_RPGStats::TickPoiseRecovery()
{
	if (CurrentPoise >= MaxPoise)
	{
		// 회복이 완료되면 타이머 중지
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(PoiseRecoveryTimerHandle);
		}
		return;
	}

	RecoverPoise(PoiseRecoveryRate);
}

float UBP_RPGStats::CalculateTotalScaling(EInnerAttributeType AttributeType)
{
	// 기본 스탯 값
	float AttributeValue = 0.0f;

	switch (AttributeType)
	{
	case EInnerAttributeType::Strength:
		AttributeValue = BaseAttributes.Strength;
		break;
	case EInnerAttributeType::Dexterity:
		AttributeValue = BaseAttributes.Dexterity;
		break;
	case EInnerAttributeType::Intelligence:
		AttributeValue = BaseAttributes.Intelligence;
		break;
	case EInnerAttributeType::Vitality:
		AttributeValue = BaseAttributes.Vitality;
		break;
	default:
		return 1.0f;
	}

	// 기본값(10)에서 얼마나 벗어났는지 비율 계산
	float AttributeRatio = AttributeValue / 10.0f;

	// 모든 장비의 해당 스탯에 대한 보정치 계산
	float TotalScaling = 1.0f;

	for (UDA_InnerEquipment* Gear : EquippedGear)
	{
		if (!Gear || !Gear->ScalingMap.Contains(AttributeType))
			continue;

		EStatScaling GearScaling = Gear->ScalingMap[AttributeType];
		float ScalingMultiplier = UDA_InnerEquipment::ScalingToMultiplier(GearScaling);

		// 보정치 누적
		if (ScalingMultiplier > 0.0f)
		{
			TotalScaling *= ScalingMultiplier;
		}
	}

	// 최종 보정: AttributeRatio * TotalScaling
	float FinalScaling = AttributeRatio * TotalScaling;

	return FMath::Max(0.0f, FinalScaling);
}

void UBP_RPGStats::BroadcastStatsChange()
{
	OnStatsChanged.Broadcast(BaseAttributes, MaxPoise);
}
