// Copyright 2024, Korstian Project. All Rights Reserved.

#include "Player/PlayerSoulCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"

// Parent components
#include "Movement/MovementLogicComponent.h"
#include "Movement/ParkourComponent.h"
#include "Combat/CombatComponent.h"


APlayerSoulCharacter::APlayerSoulCharacter()
{
	// --- CAMERA SETUP ---
	// 컨트롤러 회전 시 캐릭터는 회전하지 않도록 설정합니다. 카메라는 영향을 받습니다.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 캐릭터 이동 방향을 바라보도록 설정
	GetCharacterMovement()->bOrientRotationToMovement = true; 	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	// 카메라 붐(스프링암) 생성: 충돌 시 플레이어 방향으로 당겨집니다.
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.0f; 
	SpringArm->bUsePawnControlRotation = true; // 컨트롤러 회전에 따라 암(arm)이 회전하도록 설정

	// 팔로우 카메라 생성
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); // 카메라를 스프링암 끝에 부착
	FollowCamera->bUsePawnControlRotation = false; // 카메라는 암에 대해 상대적으로 회전하지 않음
}

void APlayerSoulCharacter::BeginPlay()
{
	Super::BeginPlay();

	// --- ENHANCED INPUT ---
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}


void APlayerSoulCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		check(MovementLogicComp);
		check(CombatComp);
		
		// --- MOVEMENT ---
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerSoulCharacter::Move);
		}
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerSoulCharacter::OnJumpPressed);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, MovementLogicComp.Get(), &UMovementLogicComponent::StartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, MovementLogicComp.Get(), &UMovementLogicComponent::StopSprint);
		}
		if (DodgeAction)
		{
			EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, MovementLogicComp.Get(), &UMovementLogicComponent::Dodge);
		}
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, MovementLogicComp.Get(), &UMovementLogicComponent::ToggleCrouch);
		}

		// --- COMBAT ---
		if (LightAttackAction)
		{
			EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, CombatComp.Get(), &UCombatComponent::LightAttack);
		}
		if (GuardAction)
		{
			EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Started, CombatComp.Get(), &UCombatComponent::StartGuard);
			EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Completed, CombatComp.Get(), &UCombatComponent::StopGuard);
		}
		
		// --- CAMERA ---
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerSoulCharacter::Look);
		}
	}
}

void APlayerSoulCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(ForwardDirection, MovementVector.Y);
	
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerSoulCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(-LookAxisVector.Y);
	}
}

void APlayerSoulCharacter::OnJumpPressed()
{
	if (ParkourComp && ParkourComp->TryParkour())
	{
		return;
	}
	Jump();
}
