// Ghost in the Monitor - Inner World Tank Movement System
// BPC_TankMovement.h
// 탱크 컨트롤 이동 로직: 가속/감속, 회전

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "BPC_TankMovement.generated.h"

// Forward Declarations
class ACharacter;
class UAnimMontage;

/**
 * BPC_TankMovement
 * 
 * Knight 캐릭터의 탱크식 이동 로직을 담당.
 * 
 * 특징:
 * - 가속/감속 보간으로 부드러운 이동감 제공
 * - 후진 시 이동 속도 감소
 * - 제자리 회전 (Strafe 불가)
 * - Q 키로 180도 회전 몽타주 재생
 * 
 * 입력 시스템과의 연동:
 * - Input_Move() : Move Axis 입력
 * - Input_Rotate() : Look/Rotate Axis 입력
 * - PerformQuickTurn() : Q 키 입력
 */
UCLASS(ClassGroup = (Movement), meta = (BlueprintSpawnableComponent))
class UEFRAMEWORKTEST_API UBPC_TankMovement : public UActorComponent
{
	GENERATED_BODY()

public:
	UBPC_TankMovement();

	/**
	 * 최대 이동 속도 (Unreal Units per second)
	 * 
	 * Default: 400.0
	 * 권장값: 300-600 (탱크식 이동이므로 그리 빠르지 않음)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxMoveSpeed;

	/**
	 * 회전 속도 (Degrees per second)
	 * 
	 * Default: 180.0
	 * 권장값: 90-270 (탱크식 회전)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float TurnRate;

	/**
	 * 후진 속도 배율 (0.0 ~ 1.0)
	 * 
	 * Default: 0.6
	 * 공식: ActualBackwardSpeed = MaxMoveSpeed * BackwardsPenalty
	 * 권장값: 0.4-0.7 (후진이 전진보다 느림)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BackwardsPenalty;

	/**
	 * 가속도 보간 계수 (RInterpSpeed 값)
	 * 
	 * Default: 10.0
	 * 공식: FMath::FInterpTo(CurrentAxis, TargetAxis, DeltaTime, Acceleration)
	 * 권장값: 5.0-15.0 (값이 클수록 빠른 가속)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Acceleration;

	/**
	 * Quick Turn 몽타주
	 * 
	 * 180도 회전 애니메이션. 몽타주 종료(AnimNotify) 시점에
	 * 실제 액터 회전이 반전됨.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	UAnimMontage* QuickTurnMontage;

	/**
	 * 현재 이동 축 입력값 (보간 전)
	 * Range: -1.0 ~ 1.0 (음수: 후진, 양수: 전진)
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float CurrentAxisValue;

	/**
	 * 목표 이동 축 입력값 (직접 입력)
	 * Range: -1.0 ~ 1.0
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float TargetAxisValue;

	/**
	 * Quick Turn 실행 중 여부
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsPerformingQuickTurn;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/**
	 * Input_Move
	 * 앞뒤 이동 입력 처리
	 * 
	 * 로직:
	 * 1. TargetAxisValue = AxisValue 설정
	 * 2. CurrentAxisValue를 TargetAxisValue로 보간 (FInterpTo)
	 * 3. 후진(CurrentAxisValue < 0) 시 BackwardsPenalty 적용
	 * 4. CharacterMovement->AddMovementInput() 호출
	 * 
	 * @param AxisValue - 입력값 (-1.0 ~ 1.0)
	 *                    양수: 전진, 음수: 후진, 0: 정지
	 * 
	 * 예제 (PlayerController Input Mapping):
	 *   MoveForward Input -> Input_Move(+1.0)
	 *   MoveBackward Input -> Input_Move(-1.0)
	 *   Release Input -> Input_Move(0.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Input_Move(float AxisValue);

	/**
	 * Input_Rotate
	 * 회전 입력 처리
	 * 
	 * 로직:
	 * 1. 회전 각도 = AxisValue * TurnRate * DeltaTime
	 * 2. SetActorRotation() 사용하여 제자리 회전
	 * (CharacterMovement.bUseControllerDesiredRotation = false)
	 * 
	 * @param AxisValue - 입력값 (-1.0 ~ 1.0)
	 *                    양수: 시계방향, 음수: 반시계방향, 0: 정지
	 * 
	 * 예제 (PlayerController Input Mapping):
	 *   TurnLeft Input -> Input_Rotate(-1.0)
	 *   TurnRight Input -> Input_Rotate(+1.0)
	 *   Release Input -> Input_Rotate(0.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Input_Rotate(float AxisValue);

	/**
	 * PerformQuickTurn
	 * 180도 회전 몽타주 재생
	 * 
	 * 로직:
	 * 1. QuickTurnMontage 재생 시작
	 * 2. bIsPerformingQuickTurn = true
	 * 3. 몽타주 중간(AnimNotify 시점)에 실제 액터 회전 반전
	 * 4. 몽타주 종료 후 bIsPerformingQuickTurn = false
	 * 
	 * 입력: Q Key
	 * 
	 * @return 몽타주가 성공적으로 재생되었는지 여부
	 * 
	 * 예제:
	 *   if (PlayerInput->IsKeyPressed(EKeys::Q))
	 *   {
	 *       TankMovement->PerformQuickTurn();
	 *   }
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool PerformQuickTurn();

	/**
	 * OnQuickTurnNotify
	 * AnimNotify에서 호출될 콜백
	 * 몽타주 재생 중 실제 액터 회전 반전
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void OnQuickTurnNotify();

	/**
	 * GetMaxMovementSpeed
	 * 현재 적용되는 최대 이동 속도 조회
	 * (후진 여부에 따라 달라짐)
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetMaxMovementSpeed() const;

	/**
	 * IsMoving
	 * 현재 이동 중인지 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsMoving() const;

protected:
	/**
	 * UpdateMovement
	 * 매 프레임 이동 업데이트 처리
	 * (TickComponent에서 호출)
	 */
	void UpdateMovement(float DeltaTime);

	/**
	 * ApplyMovementInput
	 * 실제 이동 입력 적용
	 * CharacterMovement->AddMovementInput() 호출
	 */
	void ApplyMovementInput();

private:
	/** 회전 입력값 캐시 */
	float CachedRotationInput;

	/** 몽타주 완료 콜백 핸들 */
	FDelegateHandle QuickTurnMontageEndHandle;
};
