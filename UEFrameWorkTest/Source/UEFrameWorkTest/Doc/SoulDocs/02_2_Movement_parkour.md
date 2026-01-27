
---

## 1. 파쿠르 시스템 설계 원칙

* **동적 환경 인식:** `Line Trace`와 `Sphere Trace`를 사용하여 전방의 장애물 높이, 두께, 벽의 각도를 실시간으로 판별합니다.
* **스탯 기반 기동성:** '힘(Strength)'은 벽 타기 지속 시간에 영향을 주고, '민첩(Dexterity)'은 뛰어넘기 속도와 연동됩니다.
* **루트 모션 및 모션 워핑(Motion Warping):** 지형의 높이가 매번 다르므로, UE5의 **Motion Warping**을 사용하여 캐릭터의 손과 발이 정확히 지형지물에 닿도록 보정합니다.

---

## 2. 확장된 스탯 연동 (Attribute Injection)

파쿠르 액션은 `AttributeComponent`로부터 다음 수치를 주입받아 동작합니다.

| 주입 속성 (Attribute) | 영향받는 변수 | 설명 |
| --- | --- | --- |
| **ClimbSpeedRate** | `ClimbVelocity` | 벽 타기 및 렛지 이동 속도 |
| **ParkourStaminaCost** | `ActionCost` | 뛰어넘기 및 벽 타기 시 소모되는 스태미나량 |
| **Strength (힘)** | `MaxClimbTime` | 벽에 매달려 버틸 수 있는 최대 시간 (모디파이어로 연산) |
| **Dexterity (민첩)** | `VaultAnimationSpeed` | 뛰어넘기 애니메이션 재생 속도 배율 |

---

## 3. 핵심 기능별 세부 명세

### 3.1. 뛰어넘기 (Vaulting / Mantling)

허리 높이 이하의 장애물을 멈추지 않고 신속하게 넘어가는 기능입니다.

* **감지 로직:** 캐릭터 전방으로 레이캐스트를 쏴서 장애물의 높이가 `MaxStepHeight`보다 높고 `WaistHeight`보다 낮은지 확인.
* **실행:** 이동 중 점프 키 입력 시 발생. **Motion Warping**을 사용하여 장애물의 시작점과 끝점에 손과 발 위치를 동적으로 맞춤.
* **스탯 연동:** 민첩(Dexterity) 수치가 높을수록 애니메이션의 Play Rate을 높여 기민하게 동작.

### 3.2. 벽 타기 (Wall Climbing)

수직 벽면을 이동하는 기능입니다.

* **진입 조건:** 벽을 향해 점프하거나 공중에서 벽과 충돌 시 `Climbing` 상태로 전환.
* **이동 로직:** CMC(Character Movement Component)의 모드를 `MOVE_Custom` 또는 `MOVE_Flying`과 유사한 상태로 변경하고, 입력을 수직/수평 평면 이동으로 변환.
* **스태미나 제약:** 매달려 있는 동안 초당 스태미나가 소모되며, 0이 되면 강제로 낙하 상태(`Falling`)로 전환.

### 3.3. 렛지 그랩 및 포어그립 (Ledge Grab / Foregrip)

모서리에 매달리거나 매달린 상태에서 위로 올라가는 기능입니다.

* **감지 로직:** 캐릭터 머리 위쪽에서 전방으로 'ㄴ'자 형태의 트레이스를 수행하여 손으로 잡을 수 있는 '모서리(Ledge)'를 탐색.
* **행동 분기:**
* **Hanging:** 모서리를 잡고 좌우로 이동 가능.
* **Climb Up:** 위쪽 방향키 입력 시 매달린 상태에서 지면으로 올라옴 (루트 모션 애니메이션 사용).


* **전술적 연동:** 택티컬 슈팅 요소가 있다면, 한 손으로 매달린 채 권총 사격이 가능한지 여부를 `StateComponent`에서 결정.

---

## 4. 컴포넌트 구조 및 연결 (방법 C 적용)

파쿠르 로직은 독립적인 `ParkourComponent`로 분리하여 관리합니다.

```cpp
// UParkourComponent.cpp
void UParkourComponent::BeginPlay() {
    Super::BeginPlay();

    // 방법 C: 필요한 컴포넌트 자동 연결
    MovementComp = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
    AttributeComp = GetOwner()->FindComponentByClass<UAttributeComponent>();
    StateComp = GetOwner()->FindComponentByClass<UStateComponent>();

    // 스탯 변화에 따른 파쿠르 성능 업데이트 구독
    if (AttributeComp) {
        AttributeComp->OnAttributeChanged.AddDynamic(this, &UParkourComponent::SyncParkourStats);
    }
}

```

---

## 5. 애니메이션 및 비주얼 (UE5 핵심 기능)

1. **Motion Warping:** 장애물의 높이와 너비가 다를 때 애니메이션이 미끄러지지 않도록 Target 포인트를 설정합니다. (GrabPoint, LandPoint 등)
2. **IK (Inverse Kinematics):** 벽을 탈 때 손바닥과 발바닥이 벽면의 경사에 맞춰 정확히 밀착되도록 제어합니다.
3. **AnimInstance State Machine:** `IsClimbing`, `IsHanging`, `VaultType` 등의 변수를 통해 파쿠르 전용 애니메이션 레이어로 전환합니다.

---

## 6. 개발 장애물 및 고려사항

1. **복잡한 지형 충돌:** 콜리전 설정이 잘못된 메쉬(예: 복잡한 바위)에서 트레이스가 튀는 현상을 방지하기 위해 전용 `Parkour Trace Channel`을 설정하는 것이 좋습니다.
2. **카메라 워킹:** 벽에 밀착했을 때 카메라가 캐릭터 몸 안으로 들어오거나 시야를 가리는 현상을 방지하기 위해 벽 타기 시 카메라 오프셋을 동적으로 조절해야 합니다.
3. **네트워크 동기화:** 루트 모션을 사용하는 파쿠르는 멀티플레이 환경에서 위치 동기화가 어렵습니다. (현재 싱글 플레이 기준 설계 유지)

---

### 다음 단계 제안
