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


