## 1. 개발 프로젝트 개요

* **목표:** 높은 확장성과 유지보수성을 갖춘 3인칭 백뷰 컴뱃 프레임워크 구축.
* **핵심 컨셉:** 모듈형 액터 컴포넌트(Actor Component) 기반 설계, 데이터 중심(Data-Driven) 스탯 관리.
* **주요 확장성:** 근접 전투(판타지)뿐만 아니라 원거리 사격(택티컬 슈팅) 및 다양한 상태 효과를 즉시 수용할 수 있는 구조.

---

## 2. 기본 개발 원칙 (Design Philosophy)

1. **낮은 결합도, 높은 응집도:** 각 컴포넌트(전투, 이동, 스탯)는 서로의 내부 로직을 몰라야 하며, 주어진 데이터에만 반응합니다.
2. **비파괴적 데이터 변조:** 원본 스탯(Base Value)은 보존하며, 모든 변화는 모디파이어(Modifier) 리스트를 통한 연산 결과로만 도출합니다.
3. **데이터 중심 설계:** 수치, 애니메이션, 사운드 등은 코드가 아닌 **Data Asset**에서 정의하여 기획적 변경이 코드 수정을 유발하지 않게 합니다.
4. **반응형 업데이트 (Push 방식):** `Tick`에서 매번 값을 확인하지 않고, 값이 변경되었을 때 델리게이트(Delegate)를 통해 필요한 곳에만 신호를 보냅니다.

---

## 3. 공통 아키텍처: 추상화된 속성 시스템

모든 수치(이동 속도, 공격력, 재장전 속도 등)는 아래의 구조를 공통으로 사용합니다.

### 3.1. FModifier (모디파이어 구조체)

* **Value:** 수치 (예: 10.0, 1.2)
* **Type:** 연산 방식 (Additive, Multiplicative)
* **Source:** 원천 오브젝트 (장착 아이템, 버프 효과 등 - 제거 시 식별자)

### 3.2. FModifiableAttribute (변동 속성 구조체)

이 구조체는 모든 시스템에서 쓰이는 '수치의 원자'입니다.

* **BaseValue:** 초기값.
* **CurrentValue:** 모든 모디파이어가 계산된 최종 결과값 (캐싱됨).
* **ModifierList:** 적용 중인 모디파이어 배열.
* **Calculation:** `(Base + Sum(Add)) * Product(Multi)` 공식을 통해 `CurrentValue` 도출.

---

## 4. 컴포넌트 연결 아키텍처 (방법 C 적용)

각 컴포넌트가 서로를 참조할 때, 블루프린트에서의 유연성과 C++의 안정성을 동시에 챙깁니다.

### 핵심 로직 (C++ 예시)

```cpp
// BaseComponent.h
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Linkage")
TObjectPtr<UAttributeComponent> AttributeComponent;

// BaseComponent.cpp
void UBaseComponent::BeginPlay() {
    Super::BeginPlay();
    
    // 1. 블루프린트에서 수동으로 연결하지 않았다면 자동 검색 (방법 C)
    if (!AttributeComponent) {
        AttributeComponent = GetOwner()->FindComponentByClass<UAttributeComponent>();
    }
    
    // 2. 연결 성공 시 필요한 스탯 변화 구독 (Dependency Injection 완료)
    if (AttributeComponent) {
        InitializeWithAttributes();
    }
}

```

---

## 5. 데이터 흐름 및 주입 구조 (Mediator Pattern)

캐릭터 본체(BaseCharacter)는 각 컴포넌트 사이의 **데이터 중개자** 역할을 수행합니다.

1. **입력/이벤트 발생:** 아이템 장착 또는 상태 이상 발생.
2. **스탯 갱신:** `AttributeComponent`의 특정 속성에 모디파이어가 추가되고 최종값이 재계산됨.
3. **신호 전송:** `OnAttributeChanged` 델리게이트가 방송됨.
4. **데이터 주입:** `BaseCharacter`가 이 신호를 받아 해당 값이 필요한 컴포넌트(Movement, Combat 등)의 함수를 호출하여 값을 주입.
* *예: 속도가 변하면 `MovementComponent->SetMaxWalkSpeed()` 호출.*
* *예: 공격력이 변하면 `CombatComponent->UpdateDamageMultiplier()` 호출.*



---

## 6. 개발 단계 로드맵 (향후 진행 순서)

이 명세서를 바탕으로 아래 순서에 따라 상세 명세서를 작성하고 개발을 진행할 예정입니다.

1. **[2단계] 스탯 시스템 명세:** `FModifiableAttribute`의 구체적 구현 및 `AttributeComponent` 설계.
2. **[3단계] 무브먼트 시스템 명세:** 소울라이크 이동 로직(구르기, 관성)과 스탯 연동 방법.
3. **[4단계] 컴뱃 시스템 명세:** 판타지 근접 공격 및 택티컬 슈팅을 지원하는 액션/판정 시스템.
4. **[5단계] 아이템 및 장비 명세:** 인벤토리 데이터가 스탯 모디파이어로 전환되는 과정.

---