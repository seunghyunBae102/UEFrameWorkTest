// Copyright 2024, Korstian Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/SoulCharacter.h"
#include "PlayerSoulCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * @class APlayerSoulCharacter
 * @brief ASoulCharacter의 플레이어 제어 버전입니다.
 * 카메라와 플레이어 입력을 처리하는 역할을 담당합니다.
 */
UCLASS()
class UEFRAMEWORKTEST_API APlayerSoulCharacter : public ASoulCharacter
{
	GENERATED_BODY()

public:
	APlayerSoulCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	// --- INPUT (입력) ---
	/** @brief 플레이어에게 적용할 기본 입력 매핑 컨텍스트입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** @brief 이동 입력을 처리하는 InputAction 입니다. (Value Type: Axis2D) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	/** @brief 카메라(시점) 이동을 처리하는 InputAction 입니다. (Value Type: Axis2D) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	/** @brief 점프 입력을 처리하는 InputAction 입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	/** @brief 달리기 입력을 처리하는 InputAction 입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SprintAction;

	/** @brief 회피 입력을 처리하는 InputAction 입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DodgeAction;

	/** @brief 앉기 입력을 처리하는 InputAction 입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CrouchAction;

	/** @brief 약공격 입력을 처리하는 InputAction 입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LightAttackAction;

	/** @brief 가드 입력을 처리하는 InputAction 입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> GuardAction;

protected:
	// --- INPUT HANDLERS ---
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void OnJumpPressed();

private:
	/** @brief 카메라를 플레이어 뒤에 배치하고 충돌 처리를 담당하는 스프링암입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;

	/** @brief 플레이어가 세상을 보는 시점인 팔로우 카메라입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
};
