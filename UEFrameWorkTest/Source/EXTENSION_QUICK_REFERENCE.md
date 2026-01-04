# ARCHITECTURE EXTENSION QUICK REFERENCE

## Summary of New/Extended Systems

This document serves as a quick reference for the 5 extended components.

---

## 1. BPI_RPGCombat Interface (Extended)

**File:** `Interface/BPI_RPGCombat.h`

**New Method:**
```cpp
void ReceiveMetaImpact(FVector ImpactPoint, float Force, float RadialRadius);
```

**When Called:**
- Monitor gets hit by Outer World bullet
- Inner World character receives physical knockback
- Called by `BPC_PortalMechanic::FindAndApplyImpactToActors()`

**Example Implementation (in BPC_RPGStats or similar):**
```cpp
void UBP_RPGStats::ReceiveMetaImpact_Implementation(FVector ImpactPoint, float Force, float RadialRadius)
{
    // Apply poise damage based on force
    float PoiseDamage = Force * 0.5f;
    ReceivePoiseDamage(PoiseDamage);
    
    // Apply knockback
    FVector KnockbackDirection = (GetOwner()->GetActorLocation() - ImpactPoint).GetSafeNormal();
    GetOwner()->GetCharacterMovement()->AddImpulse(KnockbackDirection * Force, true);
}
```

---

## 2. Subsystem_EconomyManager (Extended with Wallet)

**File:** `Subsystem/Subsystem_EconomyManager.h`

**New Wallet Properties:**
```cpp
double CurrentFunds = 0.0;       // Current money
double InitialFunds = 500.0;     // Starting amount
double MaxFunds = 0.0;           // Limit (0 = none)
```

**New Wallet Methods:**
```cpp
bool DepositFunds(double Amount, const FString& Reason);
bool TrySpendFunds(double Amount, const FString& Reason);
double GetCurrentFunds() const;
bool CanAfford(double Amount) const;
void SetFunds(double NewAmount);
FString GetTransactionLog() const;
```

**New Events:**
```cpp
FOnFundsDeposited    // Fired: OnFundsDeposited.Broadcast(Amount, NewTotal, Reason)
FOnFundsSpent        // Fired: OnFundsSpent.Broadcast(Amount, NewTotal, Reason)
FOnInsufficientFunds // Fired: OnInsufficientFunds.Broadcast(RequestedAmount, AvailableFunds)
```

**Usage Example:**
```cpp
UGameInstance* GI = GetGameInstance();
USubsystem_EconomyManager* EconMgr = GI->GetSubsystem<USubsystem_EconomyManager>();

// Try to buy something for $50
if (EconMgr->TrySpendFunds(50.0, TEXT("ShopPurchase")))
{
    // Purchase succeeded
    GiveItemToPlayer(Item);
}
else
{
    // Not enough money
    ShowNotification(TEXT("Insufficient Funds!"));
}
```

---

## 3. DA_ShopItem Data Asset (Verified)

**File:** `Data/DA_ShopItem.h`

**Key Properties:**
```cpp
FText DisplayName;                           // "9mm Ammunition"
float BasePrice = 100.0f;                    // Base price (subject to inflation)
TSoftObjectPtr<UPrimaryDataAsset> ProductData; // Points to DA_AmmoType or DA_InnerEquipment
float DeliveryTime = 0.0f;                   // Seconds to deliver
UTexture2D* Icon = nullptr;                  // Shop UI icon
bool bInStock = true;                        // In stock?
FText Description;                           // Item description
FName Category = TEXT("General");            // Categorization
```

**Blueprint Creation:**
1. Right-click in Content Browser → Data Asset → DA_ShopItem
2. Set DisplayName: "9mm Parabellum"
3. Set BasePrice: 15.0 (per magazine)
4. Set ProductData: Select DA_9mmParabellum asset
5. Set DeliveryTime: 2.0 seconds
6. Set Icon: Select ammo icon texture
7. Set Description: "Standard 9x19mm ammunition"
8. Set Category: "Ammo"

---

## 4. BPC_PortalMechanic Component (Verified)

**File:** `Meta/BPC_PortalMechanic.h`

**Key Properties:**
```cpp
FVector2D InnerWorldMapSize = FVector2D(10000, 10000);     // World bounds
FVector InnerWorldOrigin = FVector(0, 0, 0);              // Origin
FVector2D MonitorResolution = FVector2D(1920, 1080);      // Display pixels
float RadialForceRadius = 500.0f;                          // Impact radius
```

**Key Methods:**
```cpp
void ProcessMonitorHit(const FHitResult& OuterHitResult, float ImpactForce);
FVector ConvertUVToInnerWorldLocation(FVector2D UV) const;
FVector2D ConvertPixelToUV(float PixelX, float PixelY) const;
void FindAndApplyImpactToActors(FVector CenterPoint, float ImpactForce, float SearchRadius);
```

**Event:**
```cpp
FOnMetaImpactApplied // Fired when impact applied: OnMetaImpactApplied.Broadcast(ImpactPoint, Force, Radius)
```

**Integration in ATerminal:**
```cpp
void ATerminal::OnHit(FHitResult HitResult, float ImpactForce)
{
    // Hand off to portal mechanic
    if (PortalMechanicComp)
    {
        PortalMechanicComp->ProcessMonitorHit(HitResult, ImpactForce);
    }
}
```

---

## 5. AMagazineBase Actor (NEW)

**File:** `Outer/AMagazineBase.h`

**Components:**
```cpp
USceneComponent* RootSceneComp;          // Root
UStaticMeshComponent* MeshComponent;     // 3D model
UBP_MagLogistic* MagLogisticComponent;  // Ammo manager
UBoxComponent* CollisionComponent;       // Physics/collision
```

**Key Properties:**
```cpp
FName MagazineType = TEXT("9x19mm");          // Type ID
int32 MaxCapacity = 30;                       // Max rounds
AActor* AttachedToWeapon = nullptr;           // Parent weapon
bool bIsSimulatingPhysics = false;            // Physics state
bool bIsInInventory = false;                  // Inventory state
FName MagazineID = TEXT("MAG_Unknown");       // Unique ID
```

**Key Methods:**

**Lifecycle:**
```cpp
void InitializeMagazine(UDA_AmmoType* AmmoType, int32 AmmoCount);
bool AttachToWeapon(AActor* WeaponActor, const FName& SocketName = TEXT("MagazineWell"));
bool DetachFromWeapon();
void TakeToInventory();
void DropFromInventory(FVector DropLocation, FVector DropVelocity = FVector::ZeroVector);
```

**Ammo Management:**
```cpp
int32 AddAmmo(UDA_AmmoType* Ammo, int32 Count = 1);
bool ConsumeAmmo(UDA_AmmoType*& OutAmmo);
void EmptyMagazine();
int32 GetAmmoCount() const;
float GetCurrentWeight() const;
EMagazineWeightStatus GetWeightStatus() const;
```

**State Queries:**
```cpp
bool IsEmpty() const;
bool IsFull() const;
bool IsAttachedToWeapon() const;
```

**Physics:**
```cpp
void SetPhysicsSimulation(bool bEnable);
```

**Events:**
```cpp
FOnMagazineBulletAdded      // Ammo inserted
FOnMagazineBulletRemoved    // Ammo removed
FOnMagazineAttached         // Attached to weapon
FOnMagazineDetached         // Detached from weapon
FOnTakenToInventory         // Taken by player
FOnDroppedFromInventory     // Dropped in world
```

---

## Workflow Examples

### Example 1: Player Buys Magazine at Terminal

```cpp
void ATerminal::BuyMagazine(int32 Quantity)
{
    USubsystem_EconomyManager* Econ = GetGameInstance()->GetSubsystem<USubsystem_EconomyManager>();
    UDA_ShopItem* ShopItem = LoadObject<UDA_ShopItem>(nullptr, TEXT("...ShopItem'..."));
    
    float TotalPrice = ShopItem->BasePrice * Quantity;
    
    if (Econ->TrySpendFunds(TotalPrice, TEXT("TerminalPurchase")))
    {
        // Spawn magazine actors
        for (int32 i = 0; i < Quantity; i++)
        {
            AMagazineBase* NewMag = GetWorld()->SpawnActor<AMagazineBase>();
            NewMag->InitializeMagazine(
                Cast<UDA_AmmoType>(ShopItem->ProductData.LoadSynchronous()),
                30  // Full load
            );
            NewMag->TakeToInventory();  // Add to player inventory
        }
    }
}
```

### Example 2: Player Loads Magazine into Weapon

```cpp
void AWeaponBase::EquipMagazine(AMagazineBase* Magazine)
{
    // Eject current magazine
    if (CurrentMagazine)
    {
        CurrentMagazine->DetachFromWeapon();
        CurrentMagazine->DropFromInventory(GetActorLocation());
    }
    
    // Attach new magazine
    CurrentMagazine = Magazine;
    CurrentMagazine->AttachToWeapon(this, TEXT("MagazineWell"));
    
    // Subscribe to ammo events
    CurrentMagazine->OnMagazineBulletRemoved.AddDynamic(this, &AWeaponBase::OnMagazineAmmoChanged);
}
```

### Example 3: Monitor Hit Inner Character

```cpp
void ATerminal::OnWeaponHit(FHitResult HitResult, float ImpactForce)
{
    // Convert monitor 2D hit to Inner World 3D impact
    PortalMechanic->ProcessMonitorHit(HitResult, ImpactForce);
    // This internally:
    // 1. Extracts UV from HitResult
    // 2. Converts to Inner World location
    // 3. Finds all BPI_RPGCombat actors in radius
    // 4. Calls ReceiveMetaImpact() on each
}
```

---

## Configuration Checklist

- [ ] Economy Manager initialized at game startup
- [ ] Initial funds set appropriately
- [ ] All DA_ShopItem assets created in editor
- [ ] Portal Mechanic component added to Terminal actor
- [ ] Inner World map bounds configured in PortalMechanic
- [ ] Magazine maximum capacity matches weapon specs
- [ ] All Blueprint implementations of BPI_RPGCombat include ReceiveMetaImpact logic
- [ ] Terminal references correct shop items in inventory
- [ ] Magazine ammo icons configured for UI display

---

## Common Pitfalls

❌ **DON'T:** Forget to call `InitializeMagazine()` on newly spawned magazines
✅ **DO:** Always initialize with ammo type and count

❌ **DON'T:** Cast actors to specific types; use Interface calls only
✅ **DO:** Call through `IBP_RPGCombat` interface

❌ **DON'T:** Forget to detach magazine before deleting weapon
✅ **DO:** Call `DetachFromWeapon()` first

❌ **DON'T:** Leave physics simulation on when magazine is in inventory
✅ **DO:** Call `TakeToInventory()` to disable physics

---

**Last Updated:** January 4, 2026
