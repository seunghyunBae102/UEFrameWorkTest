# EXTENSION PHASE - VISUAL ARCHITECTURE DIAGRAMS

**Date:** January 4, 2026  
**Project:** Ghost in the Monitor - Extension Phase

---

## 1. System Overview Architecture

```
┌──────────────────────────────────────────────────────────────────────┐
│                     GHOST IN THE MONITOR - EXTENDED SYSTEMS          │
└──────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────┐   ┌──────────────────────────────┐
│      OUTER WORLD (Reality)      │   │   INNER WORLD (Monitor)      │
│                                 │   │                              │
│  ┌─────────────────────────────┤   ├──────────────────────────────┐
│  │ ATerminal (Monitor)         │   │ ABP_InnerKnight (Player)     │
│  │ ├─ Shop System              │   │ ├─ BPC_RPGStats              │
│  │ ├─ Portal Mechanic ◄────────┼───┼──► ReceiveMetaImpact()      │
│  │ ├─ Weapon & Ammo            │   │ └─ Movement/Combat           │
│  │ └─ Magazine Management      │   │                              │
│  │                             │   │ Enemies (Same Interface)     │
│  ├─────────────────────────────┤   ├──────────────────────────────┤
│  │ AWeaponBase (Tactical Gun)  │   │ Physics Objects              │
│  │ ├─ Fire()                   │   │ ├─ Destructibles             │
│  │ ├─ LineTrace ─────┐         │   │ ├─ NPCs                      │
│  │ └─ Magazine Attach│         │   │ └─ Environmental             │
│  │                  │         │   │                              │
│  ├─────────────────────────────┤   ├──────────────────────────────┤
│  │ AMagazineBase (Actor)       │   │ All respond to               │
│  │ ├─ MeshComponent            │   │ IBP_RPGCombat interface:     │
│  │ ├─ BPC_MagLogistic          │   │ • TakeRPGDamage()            │
│  │ ├─ Physics/Collision        │   │ • GetCurrentHealth()         │
│  │ └─ State Management         │   │ • ReceiveMetaImpact()◄──────┼──┐
│  │    (Inventory/World/Equipped)   │ • IsAlive()                  │  │
│  │                             │   │ • GetCurrentPoise()          │  │
│  ├─────────────────────────────┤   └──────────────────────────────┘  │
│  │ Subsystem_EconomyManager    │                                     │
│  │ ├─ Wallet (CurrentFunds)    │                                     │
│  │ ├─ DepositFunds()           │                                     │
│  │ ├─ TrySpendFunds()          │                                     │
│  │ └─ OnFundsDeposited event   │                                     │
│  │                             │                                     │
│  └─────────────────────────────┘                                     │
│                                                                      │
│                                 Portal Mechanic                      │
│                                 Converts Impact◄──────────────────────┘
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

---

## 2. Data Flow: Terminal Purchase → Magazine Spawn

```
                        TERMINAL SHOP FLOW
                        ════════════════

Player opens Terminal
       │
       ▼
┌─────────────────────────────┐
│ ATerminal::OnInteract()     │
│ Shows WBP_Shop UI widget    │
└──────────────┬──────────────┘
               │
               ▼
          Player selects item
          (DA_ShopItem: "9mm Ammo" - $20)
               │
               ▼
┌─────────────────────────────────────────┐
│ ATerminal::OrderItem(ShopItem)          │
├─────────────────────────────────────────┤
│ 1. Get Economy Manager from GameInstance│
│ 2. Call Econ→TrySpendFunds(20.0, "...") │
└──────────────┬──────────────────────────┘
               │
       ┌───────┴────────┐
       │                │
    FALSE            TRUE
    (Fail)           (Success)
       │                │
       ▼                ▼
   Broadcast        Spawn Magazine
   OnInsufficientFunds    │
       │            ┌─────────────────────────┐
       │            │ SpawnActor<AMagazineBase>│
       │            └────────┬────────────────┘
       │                     │
       │                     ▼
       │            Initialize Magazine
       │            ├─ SetMagazineType("9x19mm")
       │            ├─ SetMaxCapacity(30)
       │            └─ Call InitializeMagazine(
       │                   DA_9mmParabellum, 30)
       │                     │
       │                     ▼
       │            Register Delivery Timer
       │            (DeliveryTime = 2 seconds)
       │                     │
       │                     ▼
       │            ╔═══════════════════╗
       │            ║ Delivery Complete ║
       │            ╚═════════╤═════════╝
       │                      │
       │                      ▼
       │            Call Magazine→TakeToInventory()
       │            ├─ bIsInInventory = true
       │            ├─ Hide mesh
       │            ├─ Disable physics
       │            └─ Fire OnTakenToInventory event
       │                      │
       └──────────┬───────────┘
                  │
                  ▼
          Player now owns magazine
          (Can equip to weapon later)
```

---

## 3. Portal Mechanic: Monitor Hit → Inner Impact

```
                    PORTAL PHYSICS FLOW
                    ══════════════════

Step 1: Player Fires Gun
        AWeaponBase::Fire()
              │
              ▼
        LineTrace from gun barrel
        Hits monitor surface (ATerminal)
              │
              ▼
Step 2: Extract Hit Data
        FHitResult (ImpactPoint, Normal, UV)
              │
              ▼
Step 3: Pass to Portal Mechanic
        BPC_PortalMechanic::ProcessMonitorHit(HitResult, Force)
              │
              ▼
        ┌───────────────────────────────────┐
        │ Extract UV Coordinates            │
        │ UGameplayStatics::FindCollisionUV │
        │ Returns: UV.X ∈ [0,1], UV.Y ∈ [0,1]
        └────────────┬──────────────────────┘
                     │
                     ▼
        ┌───────────────────────────────────────────┐
        │ Convert UV → Inner World Location        │
        │ FVector Location = ConvertUVToInnerWorld..│
        │   Location = InnerWorldOrigin             │
        │           + (UV.X * MapWidth * RightVec) │
        │           + (UV.Y * MapHeight * UpVec)   │
        │                                           │
        │ Example:                                  │
        │ UV = (0.5, 0.5)  [Monitor center]        │
        │ Maps to Inner World center location      │
        └────────────┬────────────────────────────┘
                     │
                     ▼
        ┌────────────────────────────────────┐
        │ Find Overlapping Combat Actors     │
        │ Sphere trace at Location with      │
        │ RadialForceRadius = 500m           │
        │ Filter: BPI_RPGCombat implementers │
        └────────────┬───────────────────────┘
                     │
                     ▼
        ┌────────────────────────────────────┐
        │ Apply Impact to Each Actor         │
        │ For each found actor:              │
        │ {                                  │
        │   Call IBP_RPGCombat::             │
        │   ReceiveMetaImpact(               │
        │     Location, Force, Radius)       │
        │   Fire OnMetaImpactApplied event   │
        │ }                                  │
        └────────────┬───────────────────────┘
                     │
                     ▼
Step 4: Inner Character Reaction
        ┌─────────────────────────────┐
        │ BPC_RPGStats (Component)    │
        │ ReceiveMetaImpact_Impl()    │
        │ {                           │
        │  PoiseDamage = Force * 0.5  │
        │  ReceivePoiseDamage(...)    │
        │  ApplyKnockback(Direction, │
        │                 Force)      │
        │  If Poise == 0:             │
        │    EnterStaggerState()      │
        │ }                           │
        └────────────┬────────────────┘
                     │
                     ▼
        ╔════════════════════════════════╗
        ║ Inner Character Takes Impact!  ║
        ║ • Poise damage applied         ║
        ║ • Knockback animation plays    ║
        ║ • Stagger triggered (if needed)║
        ╚════════════════════════════════╝
```

---

## 4. Magazine System State Machine

```
                    MAGAZINE STATE MACHINE
                    ════════════════════

                        ┌──────────┐
                        │ [SPAWNED]│
                        └────┬─────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │ InitializeMag() │
                    └────────┬────────┘
                             │
                    ╔════════╩════════╗
                    ║ WORLD STATE    ║
                    ║ [In World]     ║
                    ║ • Visible      ║
                    ║ • Physics ON   ║
                    ║ • Droppable    ║
                    ╚════════╤════════╝
                             │
                    ┌────────────────────┐
                    │ TakeToInventory() │
                    └────────┬───────────┘
                             │
                    ╔════════╩═══════════╗
                    ║ INVENTORY STATE   ║
                    ║ [Hidden/Stored]   ║
                    ║ • Invisible       ║
                    ║ • Physics OFF     ║
                    ║ • Equippable      ║
                    ╚════════╤═══════════╝
                             │
                    ┌────────────────────┐
                    │ AttachToWeapon()  │
                    └────────┬───────────┘
                             │
                    ╔════════╩══════════╗
                    ║ EQUIPPED STATE   ║
                    ║ [On Weapon]      ║
                    ║ • Visible        ║
                    ║ • Physics OFF    ║
                    ║ • Parent-locked  ║
                    ║ • Consumable     ║
                    ╚════════╤══════════╝
                             │
         ┌───────────────────┼───────────────────┐
         │                   │                   │
         ▼                   ▼                   ▼
    Weapon fires      Magazine empty       Manual eject
    ConsumeAmmo()     (fire with no ammo)   DetachFromWeapon()
         │                   │                   │
         ├─────── Still has ammo ──────┘         │
         │                                       │
         └──────────────────┬────────────────────┘
                            │
                   ┌────────────────┐
                   │ DetachFromWeapon()
                   └────────┬────────┘
                            │
                   ┌────────────────────┐
                   │ DropFromInventory()│
                   │ (if not in world)  │
                   └────────┬───────────┘
                            │
                    ┌───────────────┐
                    │ Back to World │ ◄──┐
                    │ (Physics ON)  │    │
                    └───────┬───────┘    │
                            │           │
                            └───────────┘
```

---

## 5. Component Hierarchy: AMagazineBase

```
                    AMAGAZINEBASE ACTOR
                    ══════════════════

                    AMagazineBase (Root AActor)
                            │
                ┌───────────────────────────┐
                │                           │
                ▼                           ▼
        RootSceneComponent         Component References
                │                   ├─ Owner: AWeapon*
         ┌──────┴──────┐            ├─ AttachedToWeapon: AActor*
         │             │            └─ State: bIsInInventory,
         ▼             ▼               bIsSimulatingPhysics
    Mesh       BPC_MagLogistic
    │           │
    │      ┌────┴────┬────────┐
    │      │         │        │
    │      ▼         ▼        ▼
    │   Ammo[]  Capacity  SpringTension
    │   Load[]  MaxCap    Events
    │
    ▼
UStaticMeshComponent
└─ Visual representation
   (Updated per ammo count)


                    EVENT PROPAGATION
                    ════════════════

BPC_MagLogistic Events        AMagazineBase Events
│                             │
├─ OnBulletInserted       ├─► OnMagazineBulletAdded
├─ OnBulletRemoved        ├─► OnMagazineBulletRemoved
├─ OnMagazineFull         │
└─ OnMagazineEmpty        │

AMagazineBase Lifecycle Events
├─ OnMagazineAttached (when AttachToWeapon called)
├─ OnMagazineDetached (when DetachFromWeapon called)
├─ OnTakenToInventory (when TakeToInventory called)
└─ OnDroppedFromInventory (when DropFromInventory called)
```

---

## 6. Economy System Flow

```
                        ECONOMY SYSTEM
                        ═════════════

        Subsystem_EconomyManager
        (GameInstanceSubsystem)
        
        ┌──────────────────────────────────┐
        │ Properties                       │
        ├──────────────────────────────────┤
        │ double CurrentFunds = 0.0        │
        │ double InitialFunds = 500.0      │
        │ double MaxFunds = 0.0 (unlimited)
        │ TArray<FString> TransactionHistory
        │ int32 MaxTransactions = 50       │
        └──────────────────────────────────┘
                    │
        ┌───────────┴───────────┐
        │                       │
        ▼                       ▼
    DepositFunds()         TrySpendFunds()
    (Add money)            (Deduct money)
        │                       │
        ├─ Amount > 0?          ├─ Amount > 0?
        ├─ Below MaxFunds?      ├─ CurrentFunds >= Amount?
        ├─ Add to funds         ├─ If YES:
        ├─ Log transaction      │  ├─ Subtract funds
        ├─ Fire event           │  ├─ Log transaction
        └─ Return true          │  ├─ Fire OnFundsSpent
                                │  └─ Return true
                                ├─ If NO:
                                │  ├─ Fire OnInsufficientFunds
                                │  └─ Return false

        ┌────────────────────────────────────┐
        │ Events (MulticastDelegates)        │
        ├────────────────────────────────────┤
        │ FOnFundsDeposited                  │
        │ (Amount, NewTotal, Reason)         │
        │                                    │
        │ FOnFundsSpent                      │
        │ (Amount, NewTotal, Reason)         │
        │                                    │
        │ FOnInsufficientFunds               │
        │ (RequestedAmount, AvailableFunds)  │
        └────────────────────────────────────┘

        Connected Subscribers:
        ├─ UI_Shop: Updates balance display
        ├─ Audio: Plays success/fail sound
        └─ Analytics: Logs transaction for metrics
```

---

## 7. Complete Integration Diagram

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    FULL SYSTEM INTEGRATION MAP                          │
└─────────────────────────────────────────────────────────────────────────┘

                            GAME INSTANCE
                                │
                        ┌───────┴───────┬───────┐
                        │               │       │
                        ▼               ▼       ▼
                    SubsystemA      SubsystemB  Subsystem_
                    (Other)         (Other)     EconomyMgr
                                                │
                        ┌───────────────────────┤
                        │                       │
                        ▼                       ▼
                    OUTER WORLD             Wallet
                    (Gameplay)              Funds
                        │                   │
        ┌───────────────┼───────────────┤
        │               │               │
        ▼               ▼               ▼
    ATerminal      AWeaponBase     AMagazineBase
    (Monitor)      (Gun)           (Ammo Container)
        │               │               │
        ├─ Mesh         ├─ Fire()       ├─ Mesh
        ├─ Portal       ├─ Magazine     ├─ Ammo (BPC)
        │  Mechanic     │  Attach       ├─ Physics
        │  (Portal)     ├─ LineTrace    └─ Events
        │              └─ OnHit
        │
        └─────┬────────────┬──────────────┐
              │            │              │
              ▼            ▼              ▼
         Converts      Detaches       Picks up
         UV to        Magazine       from floor
         Inner                       or inventory
              │                          │
              └──────────┬───────────────┘
                         │
                         ▼
              ┌──────────────────────┐
              │   INNER WORLD        │
              ├──────────────────────┤
              │ ABP_InnerKnight      │
              │ (Player Character)   │
              ├──────────────────────┤
              │ BPC_RPGStats         │
              │ • ReceiveMetaImpact()│
              │ • TakeRPGDamage()    │
              │ • GetCurrentPoise()  │
              ├──────────────────────┤
              │ Enemy Characters     │
              │ (Implement same      │
              │  BPI_RPGCombat)      │
              └──────────────────────┘
```

---

## 8. Class Dependency Graph

```
                        CLASS DEPENDENCIES
                        ══════════════════

        UPrimaryDataAsset
        │        │         │
        ▼        ▼         ▼
    DA_Ammo  DA_Shop  DA_Equipment
        │     Type     Item      
        │      │         │      (Data Layers)
        │      │         │
        └──────┼─────────┘
               │
               ▼
        AActor (Base)
        │       │
        │       ├─ ATerminal ─────────────┐
        │       │  ├─ BPC_PortalMechanic  │
        │       │  └─ Wallet reference    │
        │       │                         │
        │       └─ AWeaponBase            │
        │          ├─ Fire()              │
        │          └─ Magazine reference  │
        │                                 │
        │                 ┌───────────────┘
        │                 │
        │   ┌─────────────┴─────────┐
        │   │                       │
        ▼   ▼                       ▼
    AActor  AMagazineBase       (Others)
            ├─ USceneComp
            ├─ UStaticMeshComp
            ├─ UBP_MagLogistic
            └─ UBoxComp


    UGameInstanceSubsystem
            │
            ▼
    USubsystem_EconomyManager
    ├─ DepositFunds()
    ├─ TrySpendFunds()
    └─ Events

    UInterface
            │
            ▼
    BPI_RPGCombat
    ├─ TakeRPGDamage()
    ├─ ReceiveMetaImpact()
    ├─ IsAlive()
    └─ GetCurrentPoise()

    UActorComponent
    ├─ BPC_MagLogistic
    ├─ BPC_PortalMechanic
    ├─ BPC_RPGStats
    └─ (Others)
```

---

## 9. Sequence Diagram: Full Purchase & Reload

```
Player        Terminal      EconomyMgr    Magazine     Weapon
│             │             │             │             │
├─ Enter──────►│             │             │             │
│             │             │             │             │
│             │ (Show Shop) │             │             │
├─ Buy Ammo──►│             │             │             │
│             │ OrderItem() │             │             │
│             ├────────────►│ TrySpend    │             │
│             │             ├─ Check     │             │
│             │             ├─ Success   │             │
│             │             ├─────────────────────────►│ 
│             │             │             │   Spawn    │
│             │             │             │◄────────────┤
│             │             │             │             │
│             │ Initialize  │             │             │
│             ├─────────────────────────►│             │
│             │             │             │ Init 30 ammo
│             │ Delay 2s    │             │             │
│             │ (Delivery)  │             │             │
│             │             │             │             │
│◄─── Mag────┤◄─ Delivered ┤             │             │
│  Spawned   │             │             │             │
│             │             │             │             │
├─ Unequip ──►│             │             │             │
│   Weapon   │             │             │             │
│             │             │             │   Eject    │
│             │             │             │◄────────────┤
│             │             │             │             │
│   Mag      │             │             │   Detach   │
│  Dropped   │             │             │◄────────────┤
│   │        │             │             │             │
│   ▼        │             │             │             │
│ (Physics)  │             │ (Physics)   │             │
│             │             │   Enabled   │             │
│             │             │             │             │
├─ Pick up──►│             │             │             │
│   (Equip)  │             │             │             │
│             │             │             │ Attach     │
│             │             │     Attach Weapon
│             │             │     Socket  │             │
│             │             │ ┌───────────────────────►│
│             │             │ │   Attach OK   │
│             │             │◄─────────────────────────┤
│             │             │             │             │
│ [Ready to  │             │             │             │
│  Fire]     │             │             │             │
└─────────────┴─────────────┴─────────────┴─────────────┘
```

---

**End of Visual Architecture Diagrams**

These diagrams provide visual reference for system architecture, data flows, state machines, and integration points. Refer to EXTENSION_QUICK_REFERENCE.md for code examples implementing these flows.
