// Ghost in the Monitor - Inner World Combat System
// BPC_InnerCombat.cpp
// Implementation

#include "Inner/BPC_InnerCombat.h"
#include "Inner/BPC_TankMovement.h"
#include "Inner/BPC_RPGStats.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Interface/BPI_CombatEntity.h"


UBPC_InnerCombat::UBPC_InnerCombat()
	: GuardMovePenalty(0.5f)
	, ClashWindowTime(0.2f)
	, ClashPoiseDamage(30.0f) // 기본 패링 실패 강인도 데미지
	, bCanClash(false)
	, AttackMontageLight(nullptr)
	, AttackMontageHeavy(nullptr)
	, StaggerMontage(nullptr)
	, ClashSparkEffect(nullptr)
	, ClashSound(nullptr)
	, OwnerCharacter(nullptr)
	, TankMovementComponent(nullptr)
	, RPGStatsComponent(nullptr)
	, CurrentCombatState(EBSCombatState::Idle)
{
	// 이 컴포넌트는 더 이상 틱을 사용하지 않습니다. 강인도 회복은 BPC_RPGStats에서 관리합니다.
	PrimaryComponentTick.bCanEverTick = false;
}

void UBPC_InnerCombat::BeginPlay()
{
	Super::BeginPlay();

	// 소유자 및 필수 컴포넌트 캐시
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("BPC_InnerCombat: Owner is not a Character!"));
		SetComponentTickEnabled(false);
		return;
	}

	TankMovementComponent = OwnerCharacter->FindComponentByClass<UBPC_TankMovement>();
	if (!TankMovementComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_InnerCombat: Could not find BPC_TankMovement component!"));
	}

	RPGStatsComponent = OwnerCharacter->FindComponentByClass<UBP_RPGStats>();
	if (!RPGStatsComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("BPC_InnerCombat: Could not find BPC_RPGStats component!"));
		SetComponentTickEnabled(false);
		return;
	}

	// 스태거 이벤트 구독
	if (RPGStatsComponent)
	{
		RPGStatsComponent->OnStaggerState.AddDynamic(this, &UBPC_InnerCombat::HandleStagger);
	}
	
	SetCombatState(EBSCombatState::Idle);
	UE_LOG(LogTemp, Log, TEXT("BPC_InnerCombat: Initialized."));
}

bool UBPC_InnerCombat::AttemptAttack_Implementation(bool bIsHeavy)
{
	// 공격 가능 상태 확인
    if (!IsAlive_Implementation() || (CurrentCombatState != EBSCombatState::Idle && CurrentCombatState != EBSCombatState::Defending))
	{
		UE_LOG(LogTemp, Log, TEXT("BPC_InnerCombat::AttemptAttack - Cannot attack in state: %s"), *UEnum::GetValueAsString(CurrentCombatState));
		return false;
	}

	if (!OwnerCharacter || !OwnerCharacter->GetMesh() || !OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_InnerCombat::AttemptAttack - Owner or AnimInstance is not valid"));
		return false;
	}

	UAnimMontage* SelectedMontage = bIsHeavy ? AttackMontageHeavy : AttackMontageLight;
	if (!SelectedMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_InnerCombat::AttemptAttack - Attack Montage not set (Heavy=%d)"), bIsHeavy);
		return false;
	}

	// 몽타주 재생 및 종료 이벤트 바인딩
	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	const float MontageLength = AnimInstance->Montage_Play(SelectedMontage);
	if (MontageLength > 0.0f)
	{
		SetCombatState(EBSCombatState::Attacking);
		
		// 몽타주 종료 시 OnMontageEnded 호출
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &UBPC_InnerCombat::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, SelectedMontage);

		// Clash 윈도우 활성화
		bCanClash = true;
		GetWorld()->GetTimerManager().SetTimer(ClashWindowTimerHandle, this, &UBPC_InnerCombat::OnClashWindowEnd, ClashWindowTime, false);

		UE_LOG(LogTemp, Log, TEXT("BPC_InnerCombat::AttemptAttack - Started. Heavy=%d, Duration=%.2f"), bIsHeavy, MontageLength);
		return true;
	}

	return false;
}

bool UBPC_InnerCombat::CheckClash_Implementation(AActor* OtherWeapon)
{
	if (!OtherWeapon || !IsAlive_Implementation()) return false;

	if (bCanClash)
	{
		// 패링 성공
		SpawnClashEffect(OtherWeapon->GetActorLocation());
		OnClashOccurred.Broadcast(OtherWeapon, OtherWeapon->GetActorLocation());

		UE_LOG(LogTemp, Log, TEXT("BPC_InnerCombat::CheckClash - CLASH SUCCESS against %s!"), *OtherWeapon->GetName());
		return true;
	}
	
	// 패링 실패 - RPGStatsComponent를 통해 강인도 데미지를 입습니다.
	if (RPGStatsComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("BPC_InnerCombat::CheckClash - Clash failed. Applying %.1f poise damage."), ClashPoiseDamage);
		// BPC_RPGStats에 있는 함수를 직접 호출합니다.
        ReceivePoiseDamage_Implementation(ClashPoiseDamage, OtherWeapon);
	}
	return false;
}

void UBPC_InnerCombat::ToggleGuard_Implementation(bool bEnable)
{
	if (IsGuardActive_Implementation() == bEnable || !IsAlive_Implementation()) return;

	if (!TankMovementComponent || !OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_InnerCombat::ToggleGuard - Required components missing"));
		return;
	}
	
	if (bEnable)
	{
		float ReducedSpeed = TankMovementComponent->MaxMoveSpeed * GuardMovePenalty;
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = ReducedSpeed;
		SetCombatState(EBSCombatState::Defending);
		UE_LOG(LogTemp, Log, TEXT("BPC_InnerCombat::ToggleGuard - Guard ENABLED. Speed=%.1f"), ReducedSpeed);
	}
	else
	{
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = TankMovementComponent->MaxMoveSpeed;
		SetCombatState(EBSCombatState::Idle); // 가드 해제 시 항상 Idle로
		UE_LOG(LogTemp, Log, TEXT("BPC_InnerCombat::ToggleGuard - Guard DISABLED. Speed=%.1f"), TankMovementComponent->MaxMoveSpeed);
	}

	OnGuardToggled.Broadcast();
}

bool UBPC_InnerCombat::IsInCombat() const
{
	return CurrentCombatState == EBSCombatState::Attacking || CurrentCombatState == EBSCombatState::Staggered;
}

void UBPC_InnerCombat::ReceivePoiseDamage_Implementation(float PoiseDamage, AActor* Instigator)
{
    if (RPGStatsComponent)
    {
        // BPC_RPGStats에 데미지 처리를 위임합니다.
        // 이 함수가 호출되면 BPC_RPGStats가 강인도를 깎고, 0이 되면 OnStaggered를 호출해줄 것으로 기대합니다.
        RPGStatsComponent->TakePoiseDamage(PoiseDamage, Instigator);
    }
}

float UBPC_InnerCombat::GetCurrentPoise_Implementation() const
{
    if (RPGStatsComponent)
    {
        return RPGStatsComponent->GetCurrentPoise();
    }
    return 0.0f;
}

float UBPC_InnerCombat::GetMaxPoise_Implementation() const
{
    if (RPGStatsComponent)
    {
        return RPGStatsComponent->GetMaxPoise();
    }
    return 100.0f;
}

void UBPC_InnerCombat::OnStaggered_Implementation(FVector ImpactDirection)
{
    // 이미 스태거 상태이거나 죽었다면 중복 실행 방지
	if (CurrentCombatState == EBSCombatState::Staggered || !IsAlive_Implementation())
	{
		return;
	}
	
	SetCombatState(EBSCombatState::Staggered);
	OnStaggeredDelegate.Broadcast(nullptr, ImpactDirection); // TODO: 공격자 정보 넘기기

	if (StaggerMontage && OwnerCharacter && OwnerCharacter->GetMesh() && OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_InnerCombat::OnStaggered - STAGGERED! Playing montage. Impact from %s"), *ImpactDirection.ToString());
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();

		// 다른 몽타주 중지
		AnimInstance->StopAllMontages(0.25f);

		// 스태거 몽타주 재생 및 종료 이벤트 바인딩
		const float MontageLength = AnimInstance->Montage_Play(StaggerMontage);
		if (MontageLength > 0.0f)
		{
			FOnMontageEnded MontageEndedDelegate;
			MontageEndedDelegate.BindUObject(this, &UBPC_InnerCombat::OnMontageEnded);
			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, StaggerMontage);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_InnerCombat::OnStaggered - STAGGERED! But no StaggerMontage is set."));
		// 몽타주가 없으면 짧은 시간 후 강제로 상태 복구
		FTimerHandle TempHandle;
		GetWorld()->GetTimerManager().SetTimer(TempHandle, [this]() {
			if(CurrentCombatState == EBSCombatState::Staggered)
			{
				SetCombatState(EBSCombatState::Idle);
			}
		}, 1.0f, false);
	}
}

void UBPC_InnerCombat::OnDeath_Implementation()
{
    if (!IsAlive_Implementation()) return;

    SetCombatState(EBSCombatState::Dead);
    UE_LOG(LogTemp, Log, TEXT("BPC_InnerCombat::OnDeath - Character has died."));
    
    if (OwnerCharacter && OwnerCharacter->GetMesh() && OwnerCharacter->GetMesh()->GetAnimInstance())
    {
        OwnerCharacter->GetMesh()->GetAnimInstance()->StopAllMontages(0.25f);
    }
    // TODO: 죽음 애니메이션 재생 또는 래그돌 처리
}

bool UBPC_InnerCombat::IsAlive_Implementation() const
{
    return CurrentCombatState != EBSCombatState::Dead;
}

bool UBPC_InnerCombat::IsGuardActive_Implementation() const
{
	return CurrentCombatState == EBSCombatState::Defending;
}

EBSCombatState UBPC_InnerCombat::GetCurrentCombatState_Implementation() const
{
	return CurrentCombatState;
}

void UBPC_InnerCombat::OnClashWindowEnd()
{
	bCanClash = false;
	// UE_LOG(LogTemp, Log, TEXT("BPC_InnerCombat::OnClashWindowEnd - Clash window closed"));
}

void UBPC_InnerCombat::SpawnClashEffect(FVector Location)
{
	if (ClashSparkEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ClashSparkEffect, Location);
	}
	if (ClashSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ClashSound, Location);
	}
}

void UBPC_InnerCombat::SetCombatState(EBSCombatState NewState)
{
    if (CurrentCombatState == EBSCombatState::Dead || CurrentCombatState == NewState) return;

	CurrentCombatState = NewState;
	UE_LOG(LogTemp, Log, TEXT("BPC_InnerCombat::SetCombatState - Changed to state: %s"), *UEnum::GetValueAsString(NewState));
}

void UBPC_InnerCombat::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 몽타주가 중단되지 않고 정상적으로 끝났고, 캐릭터가 살아있을 때만 상태를 Idle로 변경합니다.
	if (!bInterrupted && IsAlive_Implementation())
	{
		UE_LOG(LogTemp, Log, TEXT("Montage %s ended. Returning to Idle state."), *Montage->GetName());
		SetCombatState(EBSCombatState::Idle);

		// 스태거 상태에서 벗어났음을 RPGStats에 알림
		if (RPGStatsComponent && RPGStatsComponent->IsStaggering())
		{
			RPGStatsComponent->ExitStagger();
		}
	}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Montage %s was interrupted or character is dead."), *Montage->GetName());
		}
	}
	
	void UBPC_InnerCombat::HandleStagger(FVector ImpactDirection)
	{
		OnStaggered_Implementation(ImpactDirection);
	}
	