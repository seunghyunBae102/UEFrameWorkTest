// Ghost in the Monitor - Inner World Tank Movement System
// BPC_TankMovement.cpp
// Implementation

#include "Inner/BPC_TankMovement.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"

UBPC_TankMovement::UBPC_TankMovement()
	: MaxMoveSpeed(400.0f)
	, TurnRate(180.0f)
	, BackwardsPenalty(0.6f)
	, Acceleration(10.0f)
	, QuickTurnMontage(nullptr)
	, CurrentAxisValue(0.0f)
	, TargetAxisValue(0.0f)
	, bIsPerformingQuickTurn(false)
	, CachedRotationInput(0.0f)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;
}

void UBPC_TankMovement::BeginPlay()
{
	Super::BeginPlay();

	// 소유자 캐릭터 검증
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_TankMovement: Owner is not a Character!"));
		return;
	}

	// 캐릭터 이동 컴포넌트 설정
	if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = MaxMoveSpeed;
		Movement->bUseControllerDesiredRotation = false;
		Movement->bOrientRotationToMovement = false;
		UE_LOG(LogTemp, Log, TEXT("BPC_TankMovement: Initialized. MaxSpeed=%.1f"), MaxMoveSpeed);
	}
}

void UBPC_TankMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateMovement(DeltaTime);
}

void UBPC_TankMovement::Input_Move(float AxisValue)
{
	TargetAxisValue = FMath::Clamp(AxisValue, -1.0f, 1.0f);
	UE_LOG(LogTemp, Verbose, TEXT("BPC_TankMovement::Input_Move - AxisValue=%.2f"), AxisValue);
}

void UBPC_TankMovement::Input_Rotate(float AxisValue)
{
	CachedRotationInput = AxisValue;
	UE_LOG(LogTemp, Verbose, TEXT("BPC_TankMovement::Input_Rotate - AxisValue=%.2f"), AxisValue);
}

bool UBPC_TankMovement::PerformQuickTurn()
{
	// 이미 Quick Turn 중이면 실행하지 않음
	if (bIsPerformingQuickTurn)
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_TankMovement::PerformQuickTurn - Already performing Quick Turn"));
		return false;
	}

	// 몽타주가 설정되지 않았으면 실패
	if (!QuickTurnMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_TankMovement::PerformQuickTurn - QuickTurnMontage is not set"));
		return false;
	}

	// 소유자 캐릭터 조회
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_TankMovement::PerformQuickTurn - Owner is not a Character"));
		return false;
	}

	// 몽타주 재생
	float MontageLength = OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Play(QuickTurnMontage);
	if (MontageLength > 0.0f)
	{
		bIsPerformingQuickTurn = true;

		// 몽타주 완료 이벤트 바인딩
		if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
		{
			FOnMontageEnded OnMontageEnded;
			OnMontageEnded.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
			{
				UBPC_TankMovement::OnQuickTurnNotify();
				//Lamda is good (it looks suck!  //FUck Dick SHit!!!)
			});
			// OnMontageEnded.BindDynamic(this, &UBPC_TankMovement::OnQuickTurnNotify);
			AnimInstance->Montage_SetEndDelegate(OnMontageEnded, QuickTurnMontage);
		}

		UE_LOG(LogTemp, Log, TEXT("BPC_TankMovement::PerformQuickTurn - Montage started. Duration=%.2f"), MontageLength);
		return true;
	}

	return false;
}

void UBPC_TankMovement::OnQuickTurnNotify()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("BPC_TankMovement::OnQuickTurnNotify - Owner is not a Character"));
		return;
	}

	// 액터 회전 반전 (180도)
	FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
	FRotator NewRotation = CurrentRotation + FRotator(0.0f, 180.0f, 0.0f);
	OwnerCharacter->SetActorRotation(NewRotation);

	bIsPerformingQuickTurn = false;
	UE_LOG(LogTemp, Log, TEXT("BPC_TankMovement::OnQuickTurnNotify - Rotation flipped. OldYaw=%.1f, NewYaw=%.1f"), 
		CurrentRotation.Yaw, NewRotation.Yaw);
}

float UBPC_TankMovement::GetMaxMovementSpeed() const
{
	// 후진일 경우 속도 감소
	if (CurrentAxisValue < 0.0f)
	{
		return MaxMoveSpeed * BackwardsPenalty;
	}
	return MaxMoveSpeed;
}

bool UBPC_TankMovement::IsMoving() const
{
	return !FMath::IsNearlyZero(CurrentAxisValue);
}

void UBPC_TankMovement::UpdateMovement(float DeltaTime)
{
	// CurrentAxisValue를 TargetAxisValue로 보간
	float PrevAxisValue = CurrentAxisValue;
	CurrentAxisValue = FMath::FInterpTo(CurrentAxisValue, TargetAxisValue, DeltaTime, Acceleration);

	// 회전 처리
	if (!FMath::IsNearlyZero(CachedRotationInput) && !bIsPerformingQuickTurn)
	{
		ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
		if (OwnerCharacter)
		{
			float RotationAmount = CachedRotationInput * TurnRate * DeltaTime;
			FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
			FRotator NewRotation = CurrentRotation + FRotator(0.0f, RotationAmount, 0.0f);
			OwnerCharacter->SetActorRotation(NewRotation);
		}
	}

	// 이동 입력 적용 (변경되었을 때만)
	if (!FMath::IsNearlyEqual(PrevAxisValue, CurrentAxisValue))
	{
		ApplyMovementInput();
	}
}

void UBPC_TankMovement::ApplyMovementInput()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
	if (!Movement)
	{
		return;
	}

	// 현재 축값에 따라 최대 속도 결정
	float EffectiveMaxSpeed = GetMaxMovementSpeed();
	Movement->MaxWalkSpeed = EffectiveMaxSpeed;

	// 캐릭터 정면 방향으로 이동
	FVector ForwardDirection = OwnerCharacter->GetActorForwardVector();
	Movement->AddInputVector(ForwardDirection*CurrentAxisValue); //->AddMovementInput(ForwardDirection, CurrentAxisValue);

	if (!FMath::IsNearlyZero(CurrentAxisValue))
	{
		UE_LOG(LogTemp, Verbose, TEXT("BPC_TankMovement::ApplyMovementInput - Axis=%.2f, MaxSpeed=%.1f"), 
			CurrentAxisValue, EffectiveMaxSpeed);
	}
}
