// Ghost in the Monitor - Inner World Knight Character
// ABP_InnerKnight.h
// Inner World의 플레이어 캐릭터 - Knight

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Interface/BPI_RPGCombat.h" // 새로운 RPG Combat 인터페이스 사용
#include "Interface/BPI_CombatEntity.h"
#include "ABP_InnerKnight.generated.h"

// Forward Declarations
class UInputMappingContext;
class UInputAction;
class UBPC_TankMovement;
class UBPC_InnerCombat;
class UBPC_QuickSlot;
class UBP_RPGStats; // RPG Stats 컴포넌트 전방 선언
class UDA_InnerItem;

/**
 * ABP_InnerKnight
 * Inner World의 플레이어 캐릭터 클래스.
 * 컴포넌트 컨테이너, 입력 라우팅, 인벤토리 관리 등을 담당.
 */
UCLASS()
class UEFRAMEWORKTEST_API ABP_InnerKnight : public ACharacter, public IBP_RPGCombat, public IBP_CombatEntity
{
	GENERATED_BODY()

public:
	ABP_InnerKnight();

	// ==================== 컴포넌트 ====================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBPC_TankMovement* TankMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBPC_InnerCombat* InnerCombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBPC_QuickSlot* QuickSlotComponent;

	// RPG 스탯을 모두 관리하는 핵심 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBP_RPGStats* RPGStatsComponent;
	
	// ==================== Enhanced Input System (UE5) ====================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveForwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveBackwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* TurnLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* TurnRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* HeavyAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* GuardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* QuickTurnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* UseItemAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* NextSlotAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* PreviousSlotAction;

	// ==================== 아이템/장비 ====================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	UDA_InnerItem* CurrentWeapon;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<UDA_InnerItem*> Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 MaxInventorySlots;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	bool bIsGuardPressed;

public:
	// ==================== 입력 콜백 함수 ====================

	void OnMoveForwardInput(const FInputActionValue& Value);
	void OnMoveBackwardInput(const FInputActionValue& Value);
	void OnTurnLeftInput(const FInputActionValue& Value);
	void OnTurnRightInput(const FInputActionValue& Value);
	void OnAttackInput(const FInputActionValue& Value);
	void OnHeavyAttackInput(const FInputActionValue& Value);
	void OnGuardInput(const FInputActionValue& Value);
	void OnGuardInputReleased(const FInputActionValue& Value);
	void OnQuickTurnInput(const FInputActionValue& Value);
	void OnUseItemInput(const FInputActionValue& Value);
	void OnNextSlotInput(const FInputActionValue& Value);
	void OnPreviousSlotInput(const FInputActionValue& Value);

	// ==================== 인벤토리 시스템 ====================

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItemToInventory(UDA_InnerItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemFromInventory(UDA_InnerItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetInventorySpace() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsInventoryFull() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool EquipWeapon(UDA_InnerItem* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	UDA_InnerItem* GetCurrentWeapon() const { return CurrentWeapon; }

	// ==================== IBP_RPGCombat 인터페이스 구현 ====================

	virtual void TakeRPGDamage_Implementation(const FRPGDamageInfo& DamageInfo) override;
	virtual float GetCurrentHealth_Implementation() const override;
	virtual float GetMaxHealth_Implementation() const override;
	virtual bool IsAlive_Implementation() const override;
	virtual float GetCurrentPoise_Implementation() const override;
	virtual bool IsStaggered_Implementation() const override;
	virtual void ReceiveMetaImpact_Implementation(FVector ImpactPoint, float Force, float RadialRadius) override;

	// ==================== IBP_CombatEntity 인터페이스 구현 ====================
	
	virtual void ReceivePoiseDamage_Implementation(float PoiseDamage, AActor* Instigator) override;
	virtual bool AttemptAttack_Implementation(bool bIsHeavy) override;
	virtual bool CheckClash_Implementation(AActor* OtherWeapon) override;
	virtual void ToggleGuard_Implementation(bool bEnable) override;
	virtual bool IsGuardActive_Implementation() const override;
	virtual EBSCombatState GetCurrentCombatState_Implementation() const override;
	virtual void OnStaggered_Implementation(FVector ImpactDirection) override;
	virtual void OnDeath_Implementation() override;
	

	// ==================== 기타 유틸리티 ====================

	UFUNCTION(BlueprintPure, Category = "Components")
	UBPC_TankMovement* GetTankMovementComponent() const { return TankMovementComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	UBPC_InnerCombat* GetInnerCombatComponent() const { return InnerCombatComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	UBPC_QuickSlot* GetQuickSlotComponent() const { return QuickSlotComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	UBP_RPGStats* GetRPGStatsComponent() const { return RPGStatsComponent; }
	
protected:
	void SetupInputSystem();
	void InitializeComponents();
};
