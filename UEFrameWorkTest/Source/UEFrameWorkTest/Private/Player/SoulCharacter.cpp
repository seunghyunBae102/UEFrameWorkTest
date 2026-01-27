#include "Player/SoulCharacter.h"
#include "Attributes/AttributeComponent.h"
#include "State/CharacterStateComponent.h"
#include "Movement/MovementLogicComponent.h"
#include "Movement/ParkourComponent.h"
#include "Combat/EquipmentComponent.h"
#include "Combat/CombatComponent.h"
#include "Data/WeaponDataAsset.h"
#include "MotionWarpingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/SphereComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"

ASoulCharacter::ASoulCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	AttributeComp = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComp"));
	StateComp = CreateDefaultSubobject<UCharacterStateComponent>(TEXT("StateComp"));
	MovementLogicComp = CreateDefaultSubobject<UMovementLogicComponent>(TEXT("MovementLogicComp"));
	ParkourComp = CreateDefaultSubobject<UParkourComponent>(TEXT("ParkourComp"));
	MotionWarpingComp = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComp"));
	EquipmentComp = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComp"));
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));

	ParryHitbox = CreateDefaultSubobject<USphereComponent>(TEXT("ParryHitbox"));
	ParryHitbox->SetupAttachment(RootComponent);
	ParryHitbox->SetSphereRadius(120.f);
	ParryHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ParryHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	ParryHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void ASoulCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (AttributeComp)
	{
		AttributeComp->OnPoiseDepleted.AddDynamic(this, &ASoulCharacter::OnPoiseDepleted);
	}
}

void ASoulCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASoulCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// --- ENHANCED INPUT ---
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		check(MovementLogicComp);
		check(ParkourComp);
		check(CombatComp);
		
		// --- MOVEMENT ---
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASoulCharacter::Move);
		}
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, GET_FUNCTION_NAME_CHECKED(ASoulCharacter, OnJumpPressed));
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, GET_FUNCTION_NAME_CHECKED(ACharacter, StopJumping));
		}
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, MovementLogicComp, GET_FUNCTION_NAME_CHECKED(UMovementLogicComponent, StartSprint));
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, MovementLogicComp, GET_FUNCTION_NAME_CHECKED(UMovementLogicComponent, StopSprint));
		}
		if (DodgeAction)
		{
			EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, MovementLogicComp, GET_FUNCTION_NAME_CHECKED(UMovementLogicComponent, Dodge));
		}
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, MovementLogicComp, GET_FUNCTION_NAME_CHECKED(UMovementLogicComponent, ToggleCrouch));
		}

		// --- COMBAT ---
		if (LightAttackAction)
		{
			EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, CombatComp, GET_FUNCTION_NAME_CHECKED(UCombatComponent, LightAttack));
		}
		if (GuardAction)
		{
			EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Started, CombatComp, GET_FUNCTION_NAME_CHECKED(UCombatComponent, StartGuard));
			EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Completed, CombatComp, GET_FUNCTION_NAME_CHECKED(UCombatComponent, StopGuard));
		}
		
		// --- CAMERA ---
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASoulCharacter::Look);
		}
	}
}

void ASoulCharacter::Move(const FInputActionValue& Value)
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

void ASoulCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

float ASoulCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!StateComp || StateComp->IsDead() || StateComp->IsDodging())
	{
		return 0.f;
	}

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.f)
	{
		return 0.f;
	}

	// --- Parry Logic ---
	if (StateComp->GetState() == ECharacterState::Parrying && ParryHitbox->IsOverlappingActor(DamageCauser))
	{
		UE_LOG(LogTemp, Log, TEXT("Parry Success against %s!"), *DamageCauser->GetName());
		if (ASoulCharacter* OtherCharacter = Cast<ASoulCharacter>(DamageCauser))
		{
			OtherCharacter->GetStunned();
		}
		return 0.f; // No damage taken on successful parry
	}
	
	// --- Guard Logic ---
	if (StateComp->GetState() == ECharacterState::Guarding)
	{
		UWeaponDataAsset* WeaponData = EquipmentComp->GetCurrentWeaponData();
		if (WeaponData)
		{
			const float DamageAfterMitigation = ActualDamage * (1.f - WeaponData->BlockMitigation);
			UE_LOG(LogTemp, Log, TEXT("Guarded attack! Damage reduced from %f to %f"), ActualDamage, DamageAfterMitigation);
			ActualDamage = DamageAfterMitigation;

			AttributeComp->AddModifier(EAttributeType::Stamina, FStatModifier(-WeaponData->GuardStaminaCost, EModifierType::Add, this));
		}
	}

	// Apply final damage to Health. The poise system will handle stuns automatically.
	AttributeComp->AddModifier(EAttributeType::Health, FStatModifier(-ActualDamage, EModifierType::Add, DamageCauser));

	if (AttributeComp->GetAttributeValue(EAttributeType::Health) <= 0.f)
	{
		StateComp->SetState(ECharacterState::Dead);
		// TODO: Play death montage, disable input, etc.
	}
	
	return ActualDamage;
}

void ASoulCharacter::OnJumpPressed()
{
	if (ParkourComp && ParkourComp->TryParkour())
	{
		return;
	}
	Jump();
}

void ASoulCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	// ... (fall damage logic remains the same)
}

void ASoulCharacter::SetParryHitboxEnabled(bool bIsEnabled)
{
	ParryHitbox->SetCollisionEnabled(bIsEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void ASoulCharacter::GetStunned()
{
	if (!StateComp || StateComp->IsDead() || StateComp->IsStunned())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("%s got stunned!"), *GetName());
	
	StateComp->SetState(ECharacterState::Stunned);
	if (HitReactionMontage)
	{
		PlayAnimMontage(HitReactionMontage);
	}

	// Timer to reset state after stun and reset poise
	FTimerHandle StunEndTimer;
	float StunDuration = HitReactionMontage ? HitReactionMontage->GetPlayLength() : 0.5f;
	GetWorld()->GetTimerManager().SetTimer(StunEndTimer, [this]() {
		if (StateComp && StateComp->IsStunned())
		{
			StateComp->SetState(ECharacterState::Idle);
			// Reset poise to base value after stun
			AttributeComp->RemoveModifiersBySource(this); // A simple way to reset, might need refinement
		}
	}, StunDuration, false);
}

void ASoulCharacter::OnPoiseDepleted()
{
	GetStunned();
}


