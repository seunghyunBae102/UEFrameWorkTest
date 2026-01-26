// Ghost in the Monitor - Inner World Knight Character
// ABP_InnerKnight.cpp
// Implementation

#include "Inner/ABP_InnerKnight.h"
#include "Inner/BPC_TankMovement.h"
#include "Inner/BPC_InnerCombat.h"
#include "Inner/BPC_QuickSlot.h"
#include "Inner/BPC_RPGStats.h"
#include "Data/DA_InnerItem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ABP_InnerKnight::ABP_InnerKnight()
	: MaxInventorySlots(30)
	, bIsGuardPressed(false)
{
	PrimaryActorTick.bCanEverTick = false; // 캐릭터는 특별한 로직이 없으면 틱이 필요 없음

	// 캐릭터 기본 설정
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));

	// 컴포넌트 생성
	TankMovementComponent = CreateDefaultSubobject<UBPC_TankMovement>(TEXT("TankMovement"));
	InnerCombatComponent = CreateDefaultSubobject<UBPC_InnerCombat>(TEXT("InnerCombat"));
	QuickSlotComponent = CreateDefaultSubobject<UBPC_QuickSlot>(TEXT("QuickSlot"));
	RPGStatsComponent = CreateDefaultSubobject<UBP_RPGStats>(TEXT("RPGStats")); // RPGStats 컴포넌트 생성
}

void ABP_InnerKnight::BeginPlay()
{
	Super::BeginPlay();
	InitializeComponents();
	SetupInputSystem();
	UE_LOG(LogTemp, Log, TEXT("ABP_InnerKnight: BeginPlay - Character initialized"));
}

void ABP_InnerKnight::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveForwardAction) EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &ABP_InnerKnight::OnMoveForwardInput);
		if (MoveBackwardAction) EnhancedInputComponent->BindAction(MoveBackwardAction, ETriggerEvent::Triggered, this, &ABP_InnerKnight::OnMoveBackwardInput);
		if (TurnLeftAction) EnhancedInputComponent->BindAction(TurnLeftAction, ETriggerEvent::Triggered, this, &ABP_InnerKnight::OnTurnLeftInput);
		if (TurnRightAction) EnhancedInputComponent->BindAction(TurnRightAction, ETriggerEvent::Triggered, this, &ABP_InnerKnight::OnTurnRightInput);
		if (AttackAction) EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ABP_InnerKnight::OnAttackInput);
		if (HeavyAttackAction) EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &ABP_InnerKnight::OnHeavyAttackInput);
		if (GuardAction)
		{
			EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Triggered, this, &ABP_InnerKnight::OnGuardInput);
			EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Completed, this, &ABP_InnerKnight::OnGuardInputReleased);
		}
		if (QuickTurnAction) EnhancedInputComponent->BindAction(QuickTurnAction, ETriggerEvent::Triggered, this, &ABP_InnerKnight::OnQuickTurnInput);
		if (UseItemAction) EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Triggered, this, &ABP_InnerKnight::OnUseItemInput);
		if (NextSlotAction) EnhancedInputComponent->BindAction(NextSlotAction, ETriggerEvent::Triggered, this, &ABP_InnerKnight::OnNextSlotInput);
		if (PreviousSlotAction) EnhancedInputComponent->BindAction(PreviousSlotAction, ETriggerEvent::Triggered, this, &ABP_InnerKnight::OnPreviousSlotInput);
		UE_LOG(LogTemp, Log, TEXT("ABP_InnerKnight::SetupPlayerInputComponent - Input bindings completed"));
	}
}

// ==================== 입력 콜백 함수 구현 ====================

void ABP_InnerKnight::OnMoveForwardInput(const FInputActionValue& Value) { if (TankMovementComponent) TankMovementComponent->Input_Move(Value.Get<float>()); }
void ABP_InnerKnight::OnMoveBackwardInput(const FInputActionValue& Value) { if (TankMovementComponent) TankMovementComponent->Input_Move(-Value.Get<float>()); }
void ABP_InnerKnight::OnTurnLeftInput(const FInputActionValue& Value) { if (TankMovementComponent) TankMovementComponent->Input_Rotate(-Value.Get<float>()); }
void ABP_InnerKnight::OnTurnRightInput(const FInputActionValue& Value) { if (TankMovementComponent) TankMovementComponent->Input_Rotate(Value.Get<float>()); }
void ABP_InnerKnight::OnAttackInput(const FInputActionValue& Value) { if (InnerCombatComponent) InnerCombatComponent->AttemptAttack(false); } // 경공격
void ABP_InnerKnight::OnHeavyAttackInput(const FInputActionValue& Value) { if (InnerCombatComponent) InnerCombatComponent->AttemptAttack(true); } // 강공격
void ABP_InnerKnight::OnGuardInput(const FInputActionValue& Value) { bIsGuardPressed = true; if (InnerCombatComponent) InnerCombatComponent->ToggleGuard(true); }
void ABP_InnerKnight::OnGuardInputReleased(const FInputActionValue& Value) { bIsGuardPressed = false; if (InnerCombatComponent) InnerCombatComponent->ToggleGuard(false); }
void ABP_InnerKnight::OnQuickTurnInput(const FInputActionValue& Value) { if (TankMovementComponent) TankMovementComponent->PerformQuickTurn(); }
void ABP_InnerKnight::OnUseItemInput(const FInputActionValue& Value) { if (QuickSlotComponent) QuickSlotComponent->UseSlot(); }
void ABP_InnerKnight::OnNextSlotInput(const FInputActionValue& Value) { if (QuickSlotComponent) QuickSlotComponent->ChangeSlot(QuickSlotComponent->GetCurrentSlotIndex() + 1); }
void ABP_InnerKnight::OnPreviousSlotInput(const FInputActionValue& Value) { if (QuickSlotComponent) QuickSlotComponent->ChangeSlot(QuickSlotComponent->GetCurrentSlotIndex() - 1); }

// ==================== 인벤토리 시스템 구현 ====================

bool ABP_InnerKnight::AddItemToInventory(UDA_InnerItem* Item)
{
	if (!Item || IsInventoryFull()) return false;
	Inventory.Add(Item);
	UE_LOG(LogTemp, Log, TEXT("Added item: %s (Total: %d)"), *Item->DisplayName.ToString(), Inventory.Num());
	return true;
}

bool ABP_InnerKnight::RemoveItemFromInventory(UDA_InnerItem* Item)
{
	if (!Item) return false;
	if (Inventory.Remove(Item) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Removed item: %s (Total: %d)"), *Item->DisplayName.ToString(), Inventory.Num());
		return true;
	}
	return false;
}

int32 ABP_InnerKnight::GetInventorySpace() const { return MaxInventorySlots - Inventory.Num(); }
bool ABP_InnerKnight::IsInventoryFull() const { return Inventory.Num() >= MaxInventorySlots; }

bool ABP_InnerKnight::EquipWeapon(UDA_InnerItem* Weapon)
{
	if (!Weapon || !Weapon->IsWeapon()) return false;
	CurrentWeapon = Weapon;
	UE_LOG(LogTemp, Log, TEXT("Equipped: %s"), *Weapon->DisplayName.ToString());
	return true;
}

// ==================== IBP_RPGCombat 인터페이스 구현 ====================

void ABP_InnerKnight::TakeRPGDamage_Implementation(const FRPGDamageInfo& DamageInfo)
{
	if (RPGStatsComponent)
	{
		// 모든 데미지 처리를 RPGStatsComponent에 위임합니다.
		RPGStatsComponent->TakeHealthDamage(DamageInfo.PhysicalDamage, DamageInfo.MagicDamage, DamageInfo.Attacker);
		RPGStatsComponent->TakePoiseDamage(DamageInfo.PoiseDamage, DamageInfo.Attacker);
		
		UE_LOG(LogTemp, Verbose, TEXT("ABP_InnerKnight::TakeRPGDamage - Delegating to RPGStatsComponent."));
	}
}

float ABP_InnerKnight::GetCurrentHealth_Implementation() const
{
	return RPGStatsComponent ? RPGStatsComponent->GetCurrentHealth() : 0.0f;
}

float ABP_InnerKnight::GetMaxHealth_Implementation() const
{
	return RPGStatsComponent ? RPGStatsComponent->GetMaxHealth() : 0.0f;
}

bool ABP_InnerKnight::IsAlive_Implementation() const
{
	return RPGStatsComponent ? RPGStatsComponent->IsAlive() : false;
}

float ABP_InnerKnight::GetCurrentPoise_Implementation() const
{
	return RPGStatsComponent ? RPGStatsComponent->GetCurrentPoise() : 0.0f;
}

bool ABP_InnerKnight::IsStaggered_Implementation() const
{
	return RPGStatsComponent ? RPGStatsComponent->IsStaggering() : false;
}

void ABP_InnerKnight::ReceiveMetaImpact_Implementation(FVector ImpactPoint, float Force, float RadialRadius)
{
	// TODO: Outer World로부터의 충격 구현
	UE_LOG(LogTemp, Log, TEXT("ABP_InnerKnight::ReceiveMetaImpact - Received meta impact, but not implemented yet."));
}

// ==================== 초기화 헬퍼 함수 ====================

void ABP_InnerKnight::SetupInputSystem()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultInputMappingContext)
			{
				Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
			}
		}
	}
}

void ABP_InnerKnight::InitializeComponents()
{
	// 모든 핵심 컴포넌트가 유효한지 확인
	if (!TankMovementComponent || !InnerCombatComponent || !QuickSlotComponent || !RPGStatsComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("ABP_InnerKnight::InitializeComponents - One or more essential components are missing!"));
		return;
	}

	Inventory.Reserve(MaxInventorySlots);
	UE_LOG(LogTemp, Log, TEXT("ABP_InnerKnight::InitializeComponents - All components checked."));
}
