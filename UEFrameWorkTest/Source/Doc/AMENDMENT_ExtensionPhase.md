# EXTENDED TECHNICAL SPECIFICATION - AMENDMENT

**Project:** Ghost in the Monitor (Extension Phase)
**Amendment Date:** January 4, 2026
**Baseline:** CoreSpec.md, InnerSpec.md, OutterSpec.md, MetaAndEconomicSpec.md

---

## Overview

This amendment documents the architectural extensions and refinements to support:
1. **Portal Physics Integration** - Monitor hits translate to Inner World impacts
2. **Manual Economy System** - Double-precision wallet with transaction tracking
3. **Actor-Based Magazines** - Detachable, droppable, stackable magazine system

---

## AMENDMENT 1: Core Enums & Interfaces (CoreSpec.md)

### Added to BPI_RPGCombat Interface

```cpp
/**
 * Monitor-based physical impact reception
 * 
 * 역할: Outer World 사격이 Inner World로 영향을 미칠 때의 물리 충격 처리
 * 
 * @param ImpactPoint: 충격점 (Inner World 좌표)
 * @param Force: 물리적 힘의 크기 (Impulse 단위)
 * @param RadialRadius: 영향 반경 (미터)
 * 
 * 로직:
 * 1. Force를 강인도 피해로 변환 (약 Force * 0.5)
 * 2. 충격점 방향으로 액터에 임펄스 적용
 * 3. 강인도가 0 이하면 Stagger 상태 전환
 * 4. 다중 충격 시 누적 효과 가능
 */
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RPGCombat")
void ReceiveMetaImpact(FVector ImpactPoint, float Force, float RadialRadius);
```

---

## AMENDMENT 2: Economy System (MetaAndEconomicSpec.md)

### Extended: Subsystem_EconomyManager

#### A. Wallet System (NEW)

**Purpose:** Global player funds management (dual-track with Inner World gold system)

**Architecture:**
- GameInstanceSubsystem scope (survives map transitions)
- Double-precision for currency calculations
- Transaction history logging (50-event buffer)

**Properties:**
```cpp
// Current account balance (USD equivalent)
double CurrentFunds = 0.0;

// Starting balance (reset value)
double InitialFunds = 500.0;

// Maximum holding limit (0 = unlimited)
double MaxFunds = 0.0;
```

**Methods:**

| Method | Signature | Returns | Effect |
|--------|-----------|---------|--------|
| `DepositFunds` | `(double Amount, const FString& Reason)` | `bool` | Add money, fire OnFundsDeposited |
| `TrySpendFunds` | `(double Amount, const FString& Reason)` | `bool` | Deduct if possible, fire OnFundsSpent or OnInsufficientFunds |
| `GetCurrentFunds` | `()` | `double` | Query current balance |
| `CanAfford` | `(double Amount)` | `bool` | Check if amount available |
| `SetFunds` | `(double NewAmount)` | `void` | Direct balance set (debug/init) |
| `GetTransactionLog` | `()` | `FString` | Last 50 transactions |

**Events:**

```cpp
// Fired when funds deposited
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFundsDeposited, double, Amount, double, NewTotal, FString, Reason);

// Fired when funds spent
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFundsSpent, double, Amount, double, NewTotal, FString, Reason);

// Fired when insufficient funds
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInsufficientFunds, double, RequestedAmount, double, AvailableFunds);
```

#### B. Integration Points

**Gold ↔ Funds Conversion (AGoldExchange):**
```
Inner Knight collects gold (UInnerGold)
    ↓
Brings to AGoldExchange
    ↓
AGoldExchange::OnInteract() triggers
    ↓
Calls: Econ->DepositFunds(GoldAmount * ExchangeRate, "GoldExchange")
    ↓
Player's real-world funds increase
```

**Terminal Purchase:**
```
Player reads terminal menu
    ↓
Selects DA_ShopItem (e.g., ammo for $20)
    ↓
Terminal calls: Econ->TrySpendFunds(20.0, "TerminalPurchase")
    ↓
If true: Spawn item, register delivery timer
If false: Broadcast OnInsufficientFunds event
```

---

## AMENDMENT 3: Shop System (New Addition to OutterSpec.md)

### DA_ShopItem Data Asset

**Purpose:** Catalog entry for Terminal shop

**Inheritance:** UPrimaryDataAsset

**Properties:**

```cpp
// Display name shown in Terminal UI
FText DisplayName = FText::FromString(TEXT("Ammunition (9mm)"));

// Base selling price (subject to economy inflation)
float BasePrice = 100.0f;

// Soft reference to actual product data
// Can point to UDA_AmmoType or UDA_InnerEquipment
TSoftObjectPtr<UPrimaryDataAsset> ProductData;

// Seconds until delivery after purchase
float DeliveryTime = 0.0f;

// Terminal UI icon texture
UTexture2D* Icon = nullptr;

// Is item currently in stock?
bool bInStock = true;

// Extended description
FText Description;

// Categorization (Ammo, Parts, Equipment, Consumable)
FName Category = TEXT("General");
```

**Workflow:**

```
ATerminal maintains TArray<UDA_ShopItem*> ShopInventory
    ↓
Player opens terminal UI
    ↓
UI populates from ShopInventory
    ↓
Player selects item → clicks BUY
    ↓
Terminal calls OrderItem(SelectedShopItem)
    ↓
OrderItem() calls:
  1. Econ->TrySpendFunds(ShopItem->BasePrice)
  2. If successful: Spawn magazine actor or item
  3. Register delivery timer (DeliveryTime seconds)
  4. Fire OnItemDelivered event
```

---

## AMENDMENT 4: Portal Physics System (MetaAndEconomicSpec.md → Extended)

### Component: BPC_PortalMechanic

**Purpose:** Bridge monitor 2D hits to Inner World 3D impacts

**Inheritance:** UActorComponent (attached to ATerminal or similar)

**Key Innovation:** UV Coordinate Mapping

#### A. Coordinate System

**Monitor Space (2D):**
- Pixel coordinates: (0, 0) to (Width, Height)
- Normalized UV: (0, 0) to (1, 1)
- Origin: Top-left corner (typical screen space)

**Inner World Space (3D):**
- World coordinates: configurable origin
- Forward vector: aligns with Inner World layout
- Bidirectional mapping preserves spatial relationships

#### B. Core Algorithm

**UV to World Conversion:**
```cpp
FVector ConvertUVToInnerWorldLocation(FVector2D UV) const
{
    // UV (0,0) = InnerWorldOrigin
    // UV (1,0) = InnerWorldOrigin + (InnerWorldMapSize.X * RightVector)
    // UV (0,1) = InnerWorldOrigin + (InnerWorldMapSize.Y * UpVector)
    
    return InnerWorldOrigin 
        + (UV.X * InnerWorldMapSize.X * RightVector)
        + (UV.Y * InnerWorldMapSize.Y * UpVector);
}
```

**Pixel to UV Conversion:**
```cpp
FVector2D ConvertPixelToUV(float PixelX, float PixelY) const
{
    return FVector2D(
        PixelX / MonitorResolution.X,
        PixelY / MonitorResolution.Y
    );
}
```

#### C. Hit Processing Pipeline

```
ProcessMonitorHit(HitResult, ImpactForce)
    ↓
Step 1: Extract HitResult coordinates
    └─ UGameplayStatics::FindCollisionUV(HitResult, TextureRes, OutUV)
    ↓
Step 2: Convert UV to Inner World location
    └─ ImpactLocation = ConvertUVToInnerWorldLocation(UV)
    ↓
Step 3: Find overlapping BPI_RPGCombat actors
    └─ Sphere trace at ImpactLocation with RadialForceRadius
    ↓
Step 4: Apply impact to each actor
    └─ For each found actor:
        ├─ Call IBP_RPGCombat::ReceiveMetaImpact()
        ├─ Fire OnMetaImpactApplied event
        └─ Log impact for debugging
```

#### D. Properties

```cpp
// Inner World spatial bounds
FVector2D InnerWorldMapSize = FVector2D(10000, 10000);

// World origin (where UV 0,0 maps to)
FVector InnerWorldOrigin = FVector(0, 0, 100);  // Z offset for ground level

// Monitor display resolution (pixels)
FVector2D MonitorResolution = FVector2D(1920, 1080);

// Physics impact radius in Inner World
float RadialForceRadius = 500.0f;
```

#### E. Event

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMetaImpactApplied, 
    FVector, ImpactPoint, float, Force, float, Radius);
```

---

## AMENDMENT 5: Actor-Based Magazine System (New Addition to OutterSpec.md)

### Architecture Refinement: Magazines as Actors

**Historical Issue:** Magazines were BPC_MagLogistic components on the weapon. Cannot be:
- Dropped in the world
- Placed on tables
- Detached for reloading sequences
- Managed independently

**Solution:** AMagazineBase Actor

#### A. Component Hierarchy

```
AMagazineBase (AActor)
├── RootSceneComponent (USceneComponent)
│   ├── MeshComponent (UStaticMeshComponent)
│   │   └── Skeletal or Static mesh of magazine
│   ├── MagLogisticComponent (UBP_MagLogistic)
│   │   └── Ammo stack management
│   └── CollisionComponent (UBoxComponent)
│       └── Physics collision, overlap detection
```

#### B. State Machine

```
        [WORLD]
           ↓
     [DROPPABLE]
     ↙         ↘
[INVENTORY] → [EQUIPPED]
    ↑            ↓
    └────────────┘
```

| State | bIsInInventory | bIsSimulatingPhysics | AttachedToWeapon | Visible |
|-------|---|---|---|---|
| WORLD | false | true | nullptr | YES |
| INVENTORY | true | false | nullptr | NO |
| EQUIPPED | false | false | Weapon | YES |

#### C. Lifecycle Methods

**Creation:**
```cpp
AMagazineBase* Mag = GetWorld()->SpawnActor<AMagazineBase>();
Mag->InitializeMagazine(DA_9mmParabellum, 30);  // Type, count
```

**Pick Up:**
```cpp
Mag->TakeToInventory();  // Hide, disable physics
// Mag is now in player inventory
```

**Load into Weapon:**
```cpp
Mag->AttachToWeapon(Weapon, TEXT("MagazineWell"));
// Mag is now parent-locked to weapon socket
```

**Fire (Consume Ammo):**
```cpp
UDA_AmmoType* FiredRound = nullptr;
if (Mag->ConsumeAmmo(FiredRound))
{
    // Fire logic
}
else
{
    // Magazine empty - eject
    Mag->DetachFromWeapon();
}
```

**Drop/Reload:**
```cpp
Mag->DetachFromWeapon();
Mag->DropFromInventory(DropLocation, DropVelocity);
// Mag is now in world, physics-enabled
```

#### D. Event System

```cpp
// Events broadcast when magazine state changes
FOnMagazineBulletAdded(int32 NewCount, float NewWeight);
FOnMagazineBulletRemoved(int32 NewCount, float NewWeight);
FOnMagazineAttached(AActor* WeaponActor);
FOnMagazineDetached();
FOnTakenToInventory();
FOnDroppedFromInventory(FVector Location);
```

**Event Binding (Example):**
```cpp
// Weapon subscribes to magazine changes
Magazine->OnMagazineBulletRemoved.AddDynamic(this, &AWeaponBase::UpdateUIAmmoCount);
Magazine->OnMagazineEmpty.AddDynamic(this, &AWeaponBase::OnMagazineEmpty);
```

#### E. Key Methods

**Management:**
- `InitializeMagazine(UDA_AmmoType*, int32)` - Set up ammo type + count
- `AttachToWeapon(AActor*, FName)` - Socket attachment to weapon
- `DetachFromWeapon()` - Remove from weapon
- `TakeToInventory()` - Hide in inventory
- `DropFromInventory(FVector, FVector)` - Place in world

**Ammo:**
- `AddAmmo(UDA_AmmoType*, int32)` - Bulk load rounds
- `ConsumeAmmo(UDA_AmmoType*&)` - Pop one round
- `EmptyMagazine()` - Clear all rounds
- `GetAmmoCount()` - Current round count
- `GetCurrentWeight()` - Physics weight

**Queries:**
- `IsEmpty()` / `IsFull()` - State checks
- `IsAttachedToWeapon()` - Attachment status
- `GetWeightStatus()` - EMagazineWeightStatus

#### F. Integration with Weapon

**Before (Component-based):**
```
Weapon owns MagLogisticComponent
    └─ Cannot detach
    └─ Cannot drop
    └─ Married to weapon
```

**After (Actor-based):**
```
Weapon references AMagazineBase*
    ├─ Can attach (AttachToActor)
    ├─ Can detach (DetachFromActor)
    ├─ Can drop in world
    └─ Can be swapped mid-combat
```

---

## AMENDMENT 6: System Integration Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      OUTER WORLD (Reality)                  │
│  ┌──────────────────────────────────────────────────────┐   │
│  │ ATerminal (Monitor)                                  │   │
│  │  ├─ Shop System (DA_ShopItem)                       │   │
│  │  ├─ Portal Mechanic (UV↔Inner mapping)             │   │
│  │  └─ Weapon & Magazine Management                   │   │
│  └──────────────────────────────────────────────────────┘   │
│           │                                    │              │
│    ┌──────▼────────┐                ┌────────▼──────┐        │
│    │ Monitor Hit   │                │ Shop Purchase │        │
│    │ (2D UV coords)│                │ (Funds check) │        │
│    └──────┬────────┘                └────────┬──────┘        │
│           │                                   │               │
│    ┌──────▼──────────────────────┐   ┌──────▼──────────────┐│
│    │ BPC_PortalMechanic          │   │ Economy Manager      ││
│    │ (UV→Inner conversion)        │   │ (Wallet system)      ││
│    └──────┬──────────────────────┘   └──────┬──────────────┘│
│           │                                   │               │
└───────────┼───────────────────────────────────┼───────────────┘
            │                                   │
┌───────────▼───────────────────────────────────▼───────────────┐
│                      INNER WORLD (Monitor Content)             │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ ABP_InnerKnight (Player Character)                      │  │
│  │  └─ BPC_RPGStats: ReceiveMetaImpact()                   │  │
│  │     ├─ Poise damage = Force * 0.5                       │  │
│  │     ├─ Apply knockback                                  │  │
│  │     └─ Trigger stagger on poise=0                       │  │
│  ├──────────────────────────────────────────────────────────┤  │
│  │ Enemies (Also implement BPI_RPGCombat)                  │  │
│  │  └─ Same ReceiveMetaImpact response                     │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

---

## AMENDMENT 7: Implementation Checklist

### Phase 1: Core Integration ✅

- [x] BPI_RPGCombat::ReceiveMetaImpact signature added
- [x] Subsystem_EconomyManager wallet methods implemented
- [x] DA_ShopItem data asset created
- [x] BPC_PortalMechanic UV conversion logic implemented
- [x] AMagazineBase header created

### Phase 2: Blueprint Implementation (TODO)

- [ ] Create Blueprint characters implementing ReceiveMetaImpact
- [ ] Create DA_ShopItem entries for all shop items
- [ ] Configure Terminal shop inventory
- [ ] Create BP_Magazine_9mm, BP_Magazine_5_56, etc.
- [ ] Update weapon blueprints to use AMagazineBase

### Phase 3: Game Logic Implementation (TODO)

- [ ] Implement AGoldExchange::OnInteract() with fund deposit
- [ ] Implement ATerminal::OrderItem() with economy checks
- [ ] Implement AWeaponBase firing with PortalMechanic integration
- [ ] Implement AMagazineBase.cpp with all methods
- [ ] Implement reload sequence with magazine attachment

### Phase 4: Testing (TODO)

- [ ] Unit test: UV coordinate conversions
- [ ] Integration test: Monitor hit → Inner character impact
- [ ] Economy test: Fund deposit/spending/history
- [ ] Magazine test: Attach/detach/drop/inventory
- [ ] E2E test: Full purchase→delivery→reload→fire flow

---

## AMENDMENT 8: Breaking Changes & Deprecations

### Deprecated (Still functional but not recommended):

❌ **BPC_MagLogistic as weapon component only**
✅ **Use AMagazineBase instead** (allows detach/drop)

❌ **Direct cast checks for combat damage**
✅ **Use IBP_RPGCombat interface** (already rule)

---

## AMENDMENT 9: Performance Considerations

### Economy System:
- Double precision not necessary for all values
- Consider compression for transaction log (10M→100 bytes per entry)

### Portal Mechanic:
- UV conversion is O(1) math
- Actor discovery is O(n) sphere cast - cache results if possible
- Radial force radius tuning affects CPU cost

### Magazine System:
- Actor spawn cost: create once, reuse in inventory
- Component binding: minimal overhead (direct pointers)
- Event dispatching: consider listener count on high-fire weapons

---

## References

- **Original Specs:** CoreSpec.md, InnerSpec.md, OutterSpec.md, MetaAndEconomicSpec.md
- **New Headers:** BPI_RPGCombat.h, Subsystem_EconomyManager.h, DA_ShopItem.h, BPC_PortalMechanic.h, AMagazineBase.h
- **Documentation:** EXTENSION_REPORT.md, EXTENSION_QUICK_REFERENCE.md

---

**Document Status:** COMPLETE
**Baseline Revision:** Final
**Amendment Date:** January 4, 2026
**Next Review:** After AMagazineBase.cpp implementation
