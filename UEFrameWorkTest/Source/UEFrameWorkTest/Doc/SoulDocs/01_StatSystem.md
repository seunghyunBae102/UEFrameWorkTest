---

## 1. 데이터 정의 (Data Definitions)

모든 스탯은 **'식별자(Enum)'**와 **'변동 속성(Struct)'**으로 관리됩니다.

### 1.1. EAttributeType (스탯 식별자)

캐릭터가 가질 수 있는 모든 수치를 정의합니다.

* **Vitals:** `Health`, `Stamina`, `Mana`
* **Core Stats:** `Strength`, `Dexterity`, `Intelligence`, `Fortitude`
* **Combat:** `AttackPower`, `Defense`, `CriticalChance`, `Poise` (강인도)
* **Movement (연동용):** `WalkSpeedRate`, `SprintSpeedRate`, `JumpForceRate`
* **Tactical:** `ReloadSpeedRate`, `RecoilControlRate`

### 1.2. FStatModifier (구조체)

수치를 변화시키는 최소 단위입니다.

* **float Value:** 변화량.
* **EModifierType Type:** `Add` (합산), `Multiply` (곱산).
* **UObject* Source:** 이 모디파이어를 부여한 주체 (예: 특정 검, 버프 아이템).

### 1.3. FModifiableAttribute (구조체)

개별 스탯의 데이터를 담는 컨테이너입니다.

* **float BaseValue:** 초기 기본값.
* **float CurrentValue:** (캐싱용) 최종 계산된 값.
* **TArray<FStatModifier> Modifiers:** 적용 중인 모디파이어 리스트.

---

## 2. 핵심 로직: 계산 공식 (The Formula)

확장성과 밸런스를 위해 계산 순서를 엄격히 정의합니다.

> **Note:** 모든 곱산 모디파이어는 기본 1.0에서 시작합니다. (예: 20% 증가는 1.2로 계산). 만약 '감소'라면 0.8 등으로 표현합니다.

---

## 3. Attribute Component 명세

캐릭터에 부착되는 실질적인 '두뇌' 컴포넌트입니다.

### 3.1. 주요 변수

* `TMap<EAttributeType, FModifiableAttribute> AttributeMap;`
* `bool bIsDirty;` : 재계산이 필요한지 여부.

### 3.2. 주요 인터페이스 (Public Functions)

* **`AddModifier(EAttributeType Type, FStatModifier Mod)`**: 특정 스탯에 모디파이어 추가.
* **`RemoveModifiersBySource(UObject* Source)`**: 아이템 해제 시 해당 소스가 준 모든 모디파이어 제거.
* **`GetAttributeValue(EAttributeType Type)`**: 최종 계산된 `CurrentValue` 반환.
* **`InitializeBaseStats(UDataTable* StatTable)`**: 데이터 테이블로부터 초기 스탯 설정.

### 3.3. 반응형 이벤트 (Delegates)

* **`OnAttributeChanged(EAttributeType Type, float NewValue)`**: 값이 변할 때마다 방송.
* *구독자 예시: UI(체력바), Movement(속도 변경), Combat(대미지 갱신)*



---

## 4. 방법 C 기반의 초기화 및 주입 (Initialization)

`AttributeComponent`는 독자적으로 계산하고, 결과는 필요한 곳에 **'주입'**됩니다.

1. **C++ Constructor:** `AttributeComponent`를 생성하고 기본 스탯을 세팅합니다.
2. **BeginPlay (Method C):**
* 캐릭터가 들고 있는 `CombatComponent`, `MovementComponent` 등을 찾습니다.
* `AttributeComponent`의 포인터를 각 컴포넌트에 전달(주입)합니다.


3. **Sync Logic:**
* `AttributeComponent->OnAttributeChanged`에 각 시스템의 갱신 함수를 바인딩합니다.
* *예: `WalkSpeedRate`가 변하면 `CharacterMovement->MaxWalkSpeed`를 즉시 업데이트.*



---

## 5. 개발 시 주의사항 (Edge Cases)

* **하한선(Clamping):** 계산 결과가 음수가 되지 않도록 보호 로직을 넣어야 합니다. (특히 체력, 이동 속도).
* **순환 참조 방지:** '민첩에 비례해 힘이 오르고, 힘에 비례해 민첩이 오르는' 식의 모디파이어 구성은 엔진을 멈추게 할 수 있습니다. 모디파이어 추가 시 **Depth**를 제한하거나 구조적으로 막아야 합니다.
* **스태미나 재생:** 스태미나 재생 수치(`StaminaRegen`) 역시 하나의 Attribute로 취급하여, 무거운 갑옷을 입으면 재생률 모디파이어가 0.7x가 되도록 설계하세요.

---

## 6. 다음 단계로의 연결

이 스탯 시스템이 완성되면, **[3단계: 무브먼트 시스템]**에서 이 수치들을 어떻게 실시간으로 반영하여 소울라이크 특유의 '묵직한 움직임'과 '구르기'를 구현할지 다루게 됩니다.

**이 명세서에서 수정하거나 추가하고 싶은 스탯 항목이나 기능이 있으신가요?** (예: "치명타 피해량 수치를 추가하고 싶어" 등) 없다면 바로 다음 단계인 **무브먼트 시스템**으로 넘어가겠습니다.