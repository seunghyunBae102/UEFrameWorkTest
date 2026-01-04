// GHOST IN THE MONITOR - Tactical Weapon Component
#include "Outer/BPC_TacticalWeapon.h"

UBP_TacticalWeapon::UBP_TacticalWeapon()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.TickInterval = 0.0f;
}

void UBP_TacticalWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 신뢰도 초기화
	CurrentReliability = BaseReliability;

	UE_LOG(LogTemp, Log, TEXT("BPC_TacticalWeapon::BeginPlay - Weapon initialized. Reliability: %.2f"), CurrentReliability);
}

void UBP_TacticalWeapon::PullTrigger(float& OutTimeToFix)
{
	OutTimeToFix = 0.0f;

	// 1. 약실 확인
	if (!ChamberedRound)
	{
		OnEmptyClick.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("BPC_TacticalWeapon::PullTrigger - CLICK! No round chambered."));
		return;
	}

	// 2. 무기 상태 확인
	if (MalfunctionState != EWeaponMalfunctionState::Normal)
	{
		OutTimeToFix = GetFixTimeForMalfunction(MalfunctionState);
		UE_LOG(LogTemp, Warning, TEXT("BPC_TacticalWeapon::PullTrigger - Weapon has malfunction! Type: %d, Fix time: %.0f ms"),
			static_cast<uint8>(MalfunctionState), OutTimeToFix);
		OnFired.Broadcast(false, MalfunctionState, OutTimeToFix);
		return;
	}

	// 3. 잼 발생 판정
	EWeaponMalfunctionState DeterminedMalfunction;
	if (DetermineMalfunction(ChamberedRound, DeterminedMalfunction))
	{
		MalfunctionState = DeterminedMalfunction;
		OutTimeToFix = GetFixTimeForMalfunction(MalfunctionState);
		OnMalfunctionOccurred.Broadcast(MalfunctionState, OutTimeToFix);
		OnFired.Broadcast(false, MalfunctionState, OutTimeToFix);
		UE_LOG(LogTemp, Warning, TEXT("BPC_TacticalWeapon::PullTrigger - MALFUNCTION! Type: %d, Fix time: %.0f ms"),
			static_cast<uint8>(MalfunctionState), OutTimeToFix);
		return;
	}

	// 4. 격발 성공
	UE_LOG(LogTemp, Log, TEXT("BPC_TacticalWeapon::PullTrigger - FIRED! [%s]"), *ChamberedRound->AmmoID.ToString());

	// 신뢰도 감소
	CurrentReliability -= ReliabilityDecayPerShot;
	CurrentReliability = FMath::Max(0.0f, CurrentReliability);
	UE_LOG(LogTemp, Log, TEXT("BPC_TacticalWeapon::PullTrigger - Reliability decreased to: %.2f"), CurrentReliability);

	// 이벤트 브로드캐스트
	OnFired.Broadcast(true, EWeaponMalfunctionState::Normal, 0.0f);

	// 노리쇠 왕복 예약 (게임 로직에서 처리해야 함)
	// 실제로는 애니메이션 재생 후 CycleBolt()를 호출해야 함
}

bool UBP_TacticalWeapon::CycleBolt()
{
	if (!MagazineComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_TacticalWeapon::CycleBolt - No magazine attached!"));
		OnBoltCycled.Broadcast(false);
		return false;
	}

	// 1. 현재 약실의 탄 제거 (탄피 배출)
	if (ChamberedRound)
	{
		UE_LOG(LogTemp, Log, TEXT("BPC_TacticalWeapon::CycleBolt - Ejecting round: [%s]"), *ChamberedRound->AmmoID.ToString());
		ChamberedRound = nullptr;
	}

	// 2. 탄창에서 새 탄 가져오기
	UDA_AmmoType* NewRound = nullptr;
	if (!MagazineComponent->RemoveBullet(NewRound))
	{
		UE_LOG(LogTemp, Log, TEXT("BPC_TacticalWeapon::CycleBolt - Magazine empty! No round to chamber."));
		OnBoltCycled.Broadcast(false);
		return false;
	}

	// 3. 새 탄을 약실에 장전
	ChamberedRound = NewRound;
	UE_LOG(LogTemp, Log, TEXT("BPC_TacticalWeapon::CycleBolt - Chambered round: [%s]"), *NewRound->AmmoID.ToString());

	OnBoltCycled.Broadcast(true);
	return true;
}

bool UBP_TacticalWeapon::FixMalfunction()
{
	if (MalfunctionState == EWeaponMalfunctionState::Normal)
	{
		UE_LOG(LogTemp, Log, TEXT("BPC_TacticalWeapon::FixMalfunction - No malfunction to fix!"));
		return false;
	}

	EWeaponMalfunctionState OldMalfunction = MalfunctionState;
	MalfunctionState = EWeaponMalfunctionState::Normal;

	UE_LOG(LogTemp, Log, TEXT("BPC_TacticalWeapon::FixMalfunction - Fixed malfunction type: %d"), static_cast<uint8>(OldMalfunction));

	return true;
}

void UBP_TacticalWeapon::RestoreReliability()
{
	CurrentReliability = BaseReliability;
	UE_LOG(LogTemp, Log, TEXT("BPC_TacticalWeapon::RestoreReliability - Reliability restored to: %.2f"), CurrentReliability);
}

bool UBP_TacticalWeapon::CanFire() const
{
	return ChamberedRound != nullptr && MalfunctionState == EWeaponMalfunctionState::Normal && CurrentReliability > 0.0f;
}

bool UBP_TacticalWeapon::DetermineMalfunction(UDA_AmmoType* Ammo, EWeaponMalfunctionState& OutMalfunctionState)
{
	if (!Ammo)
	{
		OutMalfunctionState = EWeaponMalfunctionState::Normal;
		return false;
	}

	// 불량탄 판정
	float MisfireRoll = FMath::FRand();  // 0.0 ~ 1.0 사이의 난수
	float AdjustedMisfireChance = Ammo->MisfireChance * (2.0f - CurrentReliability);  // 신뢰도가 낮을수록 더 잘 발생

	if (MisfireRoll < AdjustedMisfireChance)
	{
		OutMalfunctionState = EWeaponMalfunctionState::Dud;
		UE_LOG(LogTemp, Warning, TEXT("BPC_TacticalWeapon::DetermineMalfunction - DUD! (Roll: %.3f, Threshold: %.3f)"),
			MisfireRoll, AdjustedMisfireChance);
		return true;
	}

	OutMalfunctionState = EWeaponMalfunctionState::Normal;
	return false;
}

float UBP_TacticalWeapon::GetFixTimeForMalfunction(EWeaponMalfunctionState MalfunctionType) const
{
	// 각 잼 타입별 해결 시간 (밀리초)
	switch (MalfunctionType)
	{
	case EWeaponMalfunctionState::Dud:
		return 2000.0f;  // 불발: 2초 (노리쇠 당기기)
	case EWeaponMalfunctionState::StovePipe:
		return 5000.0f;  // 배출불량: 5초 (탄피 수동 제거)
	case EWeaponMalfunctionState::DoubleFeed:
		return 8000.0f;  // 급탄불량: 8초 (잡지의 탄 제거)
	default:
		return 0.0f;     // 정상
	}
}

float UBP_TacticalWeapon::GetTotalPartModifier(const FString& ModifierType) const
{
	float TotalModifier = 1.0f;

	for (const UDA_WeaponPart* Part : InstalledParts)
	{
		if (!Part)
			continue;

		if (ModifierType == TEXT("Reliability"))
		{
			TotalModifier *= Part->ReliabilityModifier;
		}
		else if (ModifierType == TEXT("Recoil"))
		{
			TotalModifier *= Part->RecoilControl;
		}
		else if (ModifierType == TEXT("Ergonomics"))
		{
			TotalModifier *= Part->ErgonomicsModifier;
		}
	}

	return TotalModifier;
}
