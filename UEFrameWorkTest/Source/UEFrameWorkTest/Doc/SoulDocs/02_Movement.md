
---

## 1. 개요 및 설계 원칙

* **기본 원칙:** 모든 이동 수치는 직접 수정하지 않고, `AttributeComponent`로부터 계산된 배율(Rate)을 주입받아 CMC의 변수(`MaxWalkSpeed` 등)를 갱신합니다.
* **상태 중심 제어:** 이동 상태(걷기, 뛰기, 구르기, 앉기)는 `StateComponent`와 연동되어 다른 액션(공격, 아이템 사용) 가능 여부를 결정합니다.
* **루트 모션(Root Motion) 활용:** 구르기(Roll)와 같은 특수 이동은 애니메이션의 이동값을 실제 좌표로 사용하는 루트 모션을 사용하여 정교한 거리감을 구현합니다.

---

## 2. 스탯 시스템과의 연동 (Data Injection)

`AttributeComponent`에서 아래 속성들의 변화가 감지되면 `OnAttributeChanged` 델리게이트를 통해 무브먼트 수치를 즉시 갱신합니다.

| 주입 스탯 (Attribute) | CMC 반영 변수 | 설명 |
| --- | --- | --- |
| **WalkSpeedRate** | `MaxWalkSpeed` | 기본 이동 속도 (기본값 600 * Rate) |
| **SprintSpeedRate** | `MaxWalkSpeed` (Sprinting 시) | 질주 속도 (기본값 900 * Rate) |
| **JumpForceRate** | `JumpZVelocity` | 점프 높이 배율 |
| **GravityRate** | `GravityScale` | 낙하 속도 및 체공 시간 제어 |

---

## 3. 핵심 이동 액션 명세

### 3.1. 질주 (Sprinting)

* **작동 로직:** 질주 키 입력 시 `State`를 `Sprinting`으로 변경하고 속도 주입.
* **스태미나 소모:** 매 초당 일정량의 스태미나를 `AttributeComponent`에서 차감.
* **중단 조건:** 스태미나가 0이 되거나, 이동 입력이 멈추거나, 공격/구르기 시 즉시 중단.

### 3.2. 구르기 / 회피 (Dodge / Roll)

소울라이크의 핵심 시스템입니다.

* **입력 방식:** 방향키 + 회피 키. (제자리 시 백스텝, 이동 시 구르기)
* **실행 로직:**
1. `StateComponent`에 '구르기 가능' 상태인지 확인.
2. `AttributeComponent`에서 즉시 스태미나 차감.
3. 구르기 애니메이션 몽타주 재생 (루트 모션 활성화).


* **무적 판정(I-Frame):** 애니메이션 노티파이(AnimNotify)를 통해 무적 시작/종료 시점을 `StateComponent`에 전달하여 대미지 판정을 무시.

### 3.3. 앉기 (Crouching)

* **작동 로직:** 토글(Toggle) 또는 홀드(Hold) 방식으로 작동.
* **효과:** `MaxWalkSpeed`를 크게 낮추고(예: 0.5배), 캡슐 컴포넌트의 높이를 절반으로 줄임.
* **확장성:** 추후 스텔스 시스템(발각 확률 감소)과 연동될 수 있도록 `State`를 명확히 분리.

---

## 4. 컴포넌트 구조 및 연결 (방법 C 적용)

### 4.1. Custom Movement Logic (ActorComponent)

이 컴포넌트는 CMC를 직접 제어하는 '컨트롤러' 역할을 합니다.

```cpp
// BeginPlay 시점 연결 로직 (방법 C)
void UMovementLogicComponent::BeginPlay() {
    // 1. 주인으로부터 CMC와 AttributeComponent 찾기
    CharacterMovement = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
    AttributeComp = GetOwner()->FindComponentByClass<UAttributeComponent>();

    // 2. 스탯 변화 구독 (주입 시작)
    if (AttributeComp) {
        AttributeComp->OnAttributeChanged.AddDynamic(this, &UMovementLogicComponent::OnStatUpdated);
    }
}

```

---

## 5. 애니메이션과의 연동 (Motion Warping & Layering)

* **Motion Warping (UE5 전용):** 구르기 중 장애물이 있거나 특정 타겟이 있을 때 이동 거리를 동적으로 보정하여 어색함을 줄입니다.
* **Linked Anim Layers:** 무기를 들었을 때와 아닐 때의 이동 애니메이션(Locomotion)을 레이어 단위로 교체하여 확장성을 확보합니다.

---

## 6. 개발 장애물 및 고려사항 (Edge Cases)

1. **스태미나 부족 시 거동:** 구르기 중 스태미나가 0이 되면 구르기가 끊기는 것이 아니라, '구르기 완료 후' 스태미나 재생 유예 시간(Penalty Cooldown)을 주어야 합니다.
2. **경직(Stun)과의 우선순위:** 피격되어 경직 상태(`Stunned`)일 때는 모든 무브먼트 입력(구르기 포함)이 `StateComponent`에 의해 차단되어야 합니다.
3. **지형 적응:** 소울라이크 특유의 경사로 이동 및 낙하 대미지 판정을 위해 CMC의 `Landed()` 이벤트를 오버라이드하여 낙하 거리에 따른 `Health` 차감 로직을 연결해야 합니다.

---

## 7. 다음 단계로의 연결

무브먼트 시스템이 확립되면, 캐릭터는 이동하고 회피할 수 있는 능력을 갖추게 됩니다. 이제 이 이동 상태를 기반으로 '언제 공격할 수 있는지', '어떻게 대미지를 주는지'를 다루는 **[4단계: 컴뱃 시스템(Combat System) 명세서]**로 넘어갑니다.

**이 무브먼트 명세에서 추가하고 싶은 특수한 이동(예: 패링 후 앞잡기를 위한 대시, 벽 타기 등)이 있으신가요?** 없다면 바로 컴뱃 시스템으로 진행하겠습니다.