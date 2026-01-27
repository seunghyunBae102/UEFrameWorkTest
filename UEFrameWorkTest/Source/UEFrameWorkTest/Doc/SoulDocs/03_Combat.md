
---

## 1. 설계 원칙

* **데이터 주도형 공격(Data-Driven Attack):** 모든 공격의 배율, 애니메이션, 스테미나 소모량은 `UWeaponDataAsset`에서 결정됩니다.
* **상태 머신과의 결합:** 모든 공격은 `StateComponent`의 허가를 받아야 하며, 실행 중에는 캐릭터를 '공격 중' 상태로 고정합니다.
* **애니메이션 노티파이(AnimNotify) 기반 판정:** 공격 판정의 시작과 끝은 코드에서 타이머를 돌리는 것이 아니라, 애니메이션의 특정 프레임(노티파이)에서 제어합니다.

---

## 2. 핵심 데이터: UWeaponDataAsset

무기별로 다른 메커니즘을 지원하기 위해 상속 구조를 가진 데이터 애셋을 사용합니다.

* **기본 정보:** 무기 메쉬, 소켓 이름, 무기 유형(검, 도끼, 창 등).
* **공격 데이터 (TArray):**
* **Montage:** 재생할 애니메이션 몽타주.
* **DamageMultiplier:** `AttackPower` 스탯에 곱해질 공격 배율.
* **StaminaCost:** 공격 시 즉시 차감될 스테미나.
* **PoiseDamage:** 적에게 입히는 강인도(경직) 수치.


* **방어 데이터:**
* **BlockMitigation:** 가드 시 대미지 감소율(0.0~1.0).
* **ParryWindow:** 패링이 유효한 시간 프레임 정보.



---

## 3. 핵심 컴포넌트 로직: CombatComponent

### 3.1. 공격 시스템 (Light / Heavy Attack)

1. **입력 처리:** `LightAttack()` 또는 `HeavyAttack()` 호출.
2. **검증:** `StateComp`에서 행동 가능 여부 확인 + `AttributeComp`에서 스테미나 충분 여부 확인.
3. **실행:** * 스테미나 즉시 차감.
* 무기 데이터 애셋에서 애니메이션 몽타주 로드 및 재생.
* `ComboIndex`를 업데이트하여 연속 입력 시 다음 콤보 애니메이션 재생 준비.



### 3.2. 방어 및 패링 (Guard / Parry)

* **가드(Guard):** * 키 홀드 시 `State`를 `Guarding`으로 변경.
* 이동 속도 저하(무브먼트 시스템에 `GuardSpeedRate` 주입).
* 피격 시 `BlockMitigation`을 적용하여 대미지 계산.


* **패링(Parry):** * 짧은 시간 동안 `State`를 `Parrying`으로 변경.
* 해당 프레임에 적의 공격이 적중하면 적에게 '경직(Stun)' 상태 주입 및 '앞잡기(Riposte)' 기회 활성화.



---

## 4. 공격 판정 시스템 (Hit Detection)

소울라이크의 정교한 판정을 위해 **Trace 기반 판정**을 권장합니다.

1. **무기 트레이스(Weapon Trace):** 무기 메쉬에 `Start`와 `End` 소켓을 배치합니다.
2. **활성화:** 애니메이션 몽타주 내 `ToggleHitbox` 노티파이가 실행되는 동안 매 프레임 `Sphere Trace` 또는 `Capsule Trace`를 수행합니다.
3. **피격 리스트:** 한 번의 스윙에 같은 적이 여러 번 맞는 것을 방지하기 위해 `TArray<AActor*> HitActors` 리스트를 만들어 공격당 한 번만 대미지를 입힙니다.

---

## 5. 스탯 및 상태 연동 (Integration)

| 컴뱃 액션 | 연동 스탯 (Attribute) | 상태 제어 (State) |
| --- | --- | --- |
| **공격력 계산** | `BaseAttackPower * StrengthRate * WeaponMultiplier` | `Attacking` 상태 부여 (회피/이동 제한) |
| **스테미나 소모** | `WeaponStaminaCost * FatigueRate` | 스테미나 부족 시 공격 불가 |
| **피격 경직** | 적의 `Poise` 스탯 vs 나의 `PoiseDamage` | `Stunned` 상태 주입 (애니메이션 재생) |
| **가드** | `Defense` 및 `GuardStaminaCost` | `Guarding` 상태 (전방 판정 강화) |

---

## 6. 컴포넌트 연결 (방법 C 적용)

```cpp
// UCombatComponent.cpp
void UCombatComponent::BeginPlay() {
    Super::BeginPlay();

    // 방법 C: 주변 컴포넌트 자동 탐색 및 링크
    AttributeComp = GetOwner()->FindComponentByClass<UAttributeComponent>();
    StateComp = GetOwner()->FindComponentByClass<UStateComponent>();
    AnimInstance = GetOwner()->FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance();

    // 스탯 변화에 따른 대미지 수치 캐싱
    if (AttributeComp) {
        AttributeComp->OnAttributeChanged.AddDynamic(this, &UCombatComponent::OnStatUpdated);
    }
}

```

---

## 7. 확장성: 택티컬 슈팅으로의 연결

근접 전투 중심이지만, 이 구조는 총기 시스템으로 쉽게 확장됩니다.

* **근접 공격:** `PerformAttack()`에서 몽타주 재생 및 트레이스 발생.
* **원거리 공격:** `PerformAttack()`에서 총기 데이터 애셋의 발사 로직(탄환 스폰 또는 라인 트레이스) 호출.
* **공통:** 두 액션 모두 `AttributeComponent`의 모디파이어(힘/민첩)를 공유하여 최종 대미지를 산출합니다.

---

## 8. 개발 장애물 및 해결책

1. **공격 중 회전 제어:** 소울라이크는 공격 애니메이션 중 적을 향해 약간 회전(Tracking)해야 합니다. 이는 **Motion Warping**의 `Skew Warp`를 사용하여 공격 시작 시점에 적 방향으로 캐릭터를 정렬시켜 해결합니다.
2. **공격 캔슬(Cancel):** 특정 프레임 이후에만 회피(구르기)로 공격을 캔슬할 수 있어야 합니다. 이는 애니메이션에 `CanCombo` 또는 `CanCancel` 노티파이를 배치하여 `StateComponent`에 신호를 보내 구현합니다.

---