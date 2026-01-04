사용자의 **GDD v2.0 (택티컬 로지스틱스 & RPG)** 기획을 바탕으로, AI(LLM)가 환각 없이 코드를 구현할 수 있도록 **구조적 명세서(Technical Design Specification)**를 작성해 드립니다.

이 문서는 **"기획서의 언어"를 "개발자의 언어(C++ Spec)"로 번역**한 것입니다. AI에게 코딩을 요청할 때, 해당 섹션을 복사해서 "Context"로 제공하면 매우 높은 정확도의 코드를 얻을 수 있습니다.

---

# 🏗️ GHOST IN THE MONITOR: Technical Specification (v2.0)

## 1. 아키텍처 개요 (Architecture Overview)

* **패턴:** Component-Based Architecture (컴포넌트 중심) + Data-Driven Design (데이터 주도)
* **핵심 원칙:**
* **No HUD Logic in Outer:** 외부 세계 로직은 절대 위젯(UI)에 의존하지 않는다. 모든 상태는 애니메이션이나 사운드로 피드백한다.
* **Strict Layering:** Inner(RPG) 로직과 Outer(FPS) 로직은 서로 다른 컴포넌트와 네임스페이스를 사용하며, 오직 `Interface`와 `GameInstance(Mediator)`를 통해서만 소통한다.



---

## 2. 데이터 레이어 명세 (Data Layer Specification)

`UPrimaryDataAsset`을 상속받아, 기획자가 에디터에서 수치를 조정할 수 있는 데이터 구조입니다.

### 2.1. Layer 1: Inner World (RPG Stats)

RPG의 깊이를 담당하는 데이터입니다.

```cpp
// [Struct] RPG 4대 스탯
USTRUCT(BlueprintType)
struct FInnerAttributes
{
    // 근력: 물리 데미지 보정, 소지 중량
    float Strength; 
    // 기량: 공속, 낙법, 치명타 보정
    float Dexterity; 
    // 지력: 아이템 효율, 약점 간파
    float Intelligence; 
    // 생명력: Max HP
    float Vitality; 
};

// [Enum] 무기 보정 등급 (Scaling)
UENUM(BlueprintType)
enum class EStatScaling : uint8
{
    S, A, B, C, D, None
};

// [DataAsset] 인게임 장비 (무기/방어구)
UCLASS()
class UDA_InnerEquipment : public UPrimaryDataAsset
{
public:
    // 기본 물리 공격력/방어력
    float BasePhysPower;
    
    // 기본 마법 공격력/방어력
    float BaseMagicPower;

    // 강인도 감쇄력 (공격 시) / 강인도 제공량 (방어구)
    float PoiseValue;

    // 스탯 보정치 (Map: Attribute -> Scaling Grade)
    // 예: {Strength : A, Dexterity : D}
    TMap<EInnerAttributeType, EStatScaling> ScalingMap; 

    // 장비 무게 (Inner 캐릭터의 구르기 속도에 영향)
    float Weight;
};

```

### 2.2. Layer 2: Outer World (Tactical Logistics)

병참 공포를 유발하는 총기 및 탄약 데이터입니다.

```cpp
// [DataAsset] 탄약 규격
UCLASS()
class UDA_AmmoType : public UPrimaryDataAsset
{
public:
    // 구경 이름 (예: "9x19mm Parabellum")
    FName CaliberName;

    // 불량탄 확률 (0.0 ~ 1.0). 높으면 Jamming 발생.
    float MisfireChance;

    // 대인 저지력 (Inner 몬스터에게 들어가는 실제 데미지)
    float FleshDamage;

    // 관통력 (모니터 유리 등 사물 관통 여부)
    float PenetrationPower;
};

// [DataAsset] 총기 부품 (Modding Part)
UCLASS()
class UDA_WeaponPart : public UPrimaryDataAsset
{
public:
    // 부품 타입 (Barrel, Receiver, Stock, Magazine)
    EWeaponPartType PartType;

    // 신뢰성 보정 (내구도 감소 속도 영향)
    float ReliabilityModifier;

    // 인체공학 보정 (ADS 속도, 소음 유발 정도 영향)
    float ErgonomicsModifier;

    // 반동 제어 보정
    float RecoilControl;
};

```

---

## 3. 인터페이스 레이어 (Interface Layer)

두 세계를 연결하고, 결합도를 낮추는 통신 규약입니다.

### 3.1. `BPI_PhysicalInteraction` (Outer World)

현실 세계의 물리적 상호작용을 담당합니다.

* `InspectItem()`:
* **역할:** 들고 있는 아이템을 자세히 관찰 (탄창 무게 가늠, 총기 약실 확인).
* **Return:** 없음 (애니메이션 재생).


* `ManipulateItem(EHandAction ActionType)`:
* **역할:** 아이템에 물리적 힘 가하기.
* **Input:** `InsertBullet` (탄 넣기), `PullBolt` (노리쇠 당기기), `ClearJam` (탄 걸림 제거).



### 3.2. `BPI_RPGCombat` (Inner World)

RPG 전투 판정을 담당합니다.

* `TakeRPGDamage(FDamageInfo Info)`:
* **역할:** 공격을 받았을 때 연산.
* **Input:** 물리/마법 데미지, **PoiseDamage(강인도 피해)**.
* **Logic:** 강인도가 0이 되면 `Stagger` 상태로 전환.



---

## 4. 로직 컴포넌트 명세 (Component Specification)

실제 게임 플레이가 돌아가는 핵심 코드 블록입니다.

### 4.1. `BPC_MagLogistic` (Outer World / 탄창 관리)

**"공포의 핵심"**입니다. 탄창 하나를 관리하는 물리적 로직입니다.

* **변수 (Properties):**
* `TArray<UDA_AmmoType*> LoadedAmmo`: 현재 탄창에 들어있는 탄알들의 배열 (Stack 구조).
* `float SpringTension`: 스프링 장력. 탄이 꽉 찰수록 삽탄 시간이 오래 걸림.
* `int32 Capacity`: 최대 용량.


* **메서드 (Methods):**
* `InsertBullet(UDA_AmmoType* Ammo)`:
* **기능:** 탄알 하나를 배열에 `Push`.
* **로직:** 삽탄 시간 소요(애니메이션) -> 성공 시 배열 추가. 긴장 상태면 실패 확률 존재.


* `RemoveBullet()`:
* **기능:** 탄알 하나를 `Pop`. (탄 빼기).


* `EstimateCount()`:
* **기능:** 정확한 숫자가 아닌, 무게에 기반한 대략적 상태 반환 (Empty, Light, Heavy, Full).





### 4.2. `BPC_TacticalWeapon` (Outer World / 총기 작동)

기능 고장(Jamming)과 발사 메커니즘을 담당합니다.

* **변수 (Properties):**
* `float CurrentReliability`: 현재 총기 신뢰도 (사격 시 감소).
* `EWeaponState MalfunctionState`: 정상, 급탄불량(FeedFail), 배출불량(EjectFail).
* `UDA_AmmoType* ChamberedRound`: 현재 약실에 들어있는 탄 1발.


* **메서드 (Methods):**
* `PullTrigger()`:
* **기능:** 격발 시도.
* **로직:** 1. `ChamberedRound` 확인. 없으면 `Click` 소리.
2. `MalfunctionState` 확인. 정상이 아니면 격발 불가.
3. `ChamberedRound->MisfireChance`와 `CurrentReliability`를 대조하여 잼 발생 여부 판정.
4. 성공 시 발사 -> `CycleBolt()` 호출.


* `CycleBolt()`:
* **기능:** 노리쇠 왕복. 탄피 배출 및 차탄 장전.
* **로직:** 탄창(`BPC_MagLogistic`)에서 `RemoveBullet`으로 탄을 가져와 약실에 넣음.


* `FixMalfunction(EActionType FixAction)`:
* **기능:** 기능 고장 처치. (예: 노리쇠를 여러 번 당겨서 걸린 탄 빼내기).





### 4.3. `BPC_RPGStats` (Inner World / 스탯 연산)

RPG 수치 계산기입니다.

* **변수 (Properties):**
* `FInnerAttributes BaseAttributes`: 기본 스탯 (힘, 민, 지, 체).
* `float CurrentPoise`: 현재 강인도.


* **메서드 (Methods):**
* `CalculateAttackPower(UDA_InnerEquipment* Weapon)`:
* **기능:** 스탯 보정치가 적용된 최종 공격력 반환.
* **공식:** `BaseDmg + (BaseDmg * ScalingGrade * AttributeValue)`.


* `RegenPoise()`:
* **기능:** 일정 시간 피격되지 않으면 강인도 회복.





---

## 5. 경제 시스템 (Economy System)

동적 물가를 관리하는 서브시스템입니다.

### 5.1. `USubsystem_EconomyManager` (GameInstance Subsystem)

* **역할:** 전역적인 물가 및 상점 재고 관리.
* **변수:**
* `float CurrentInflationIndex`: 현재 인플레이션 지수 (기본 1.0).
* `float GlobalRiskLevel`: 플레이어의 위기 상황 (죽을 뻔한 횟수 등).


* **메서드:**
* `GetDynamicPrice(UDA_MarketItem* Item)`:
* **공식:** `Item.BaseValue * Item.Scarcity * CurrentInflationIndex`.


* `AdvanceEconomyTime()`:
* **기능:** 스테이지 클리어 시 인플레이션 지수 증가 (난이도 조절).





---

## 6. AI 개발 지시용 프롬프트 (Prompt Strategy)

위 명세서를 바탕으로 AI에게 코딩을 시킬 때, 아래의 **[Context Block]**을 먼저 제공하고 명령을 내리십시오.

### 📜 AI Prompt Context (Copy & Paste)

```markdown
# Context: Project "Ghost in the Monitor" (GDD v2.0)
You are a Senior Unreal Engine 5 Developer implementing a "Dual-Reality Tactical Horror" game.
Strictly adhere to the following Technical Specifications.

## Architecture Rules
1. **Layer Separation:** - `Inner_*` classes are for the RPG Retro Game (Soulslike).
   - `Outer_*` classes are for the Realistic Tactical Shooter (Tarkov-like).
2. **Data-Driven:** Use `UPrimaryDataAsset` for ALL static data. Do not hardcode numbers.
3. **No HUD in Outer:** The Outer World player has NO HUD. Output state via Log, Animation, or Sound.

## Key Class Specifications

### 1. Outer World Weapon Logic
- **Component:** `BPC_TacticalWeapon`
- **Responsibility:** Handles Firing, Jamming, and Bolt Cycling.
- **Key Logic:**
  - `AttemptFire()`: Checks `ChamberedRound` and `MalfunctionState`. Calculates RNG for Misfire based on Ammo Quality & Gun Reliability.
  - `MalfunctionState`: Use Enum {None, StovePipe, DoubleFeed, Dud}.

### 2. Outer World Magazine Logic
- **Component:** `BPC_MagLogistic`
- **Responsibility:** Manages individual bullets physically.
- **Key Logic:**
  - `BulletsArray`: `TArray<UDA_AmmoType*>`. Acts as a Stack.
  - `InsertBullet()`: Must simulate time delay per bullet. Fails if mag is full.
  - `CheckWeight()`: Returns Enum {Empty, Light, Medium, Heavy, Full} instead of exact count.

### 3. Inner World RPG Stats
- **Component:** `BPC_RPGStats`
- **Responsibility:** Calculates derived stats from Attributes (Str, Dex, Int, Vit).
- **Key Logic:**
  - `GetScaledDamage(Weapon)`: Applies S/A/B/C/D scaling multipliers to Base Damage.
  - `PoiseSystem`: Manages Stagger threshold.

---
# Task:
(여기에 원하는 작업을 입력하세요. 예: "BPC_MagLogistic 컴포넌트의 InsertBullet 함수를 C++로 작성해줘.")

```

---

### 💡 전문가의 조언 (Implementation Advice)

1. **순서:** `UDA_AmmoType`과 `UDA_WeaponPart` 데이터 에셋 클래스부터 만드십시오. 데이터 구조가 없으면 로직을 짤 수 없습니다.
2. **테스트:** `BPC_MagLogistic`만 따로 떼어서, 빈 액터에 붙인 뒤 에디터에서 `InsertBullet`을 호출해보며 로그가 제대로 찍히는지(배열에 쌓이는지) 먼저 검증하십시오. 그래픽 없이 **로그(Log)**만으로 재미와 긴장감이 느껴져야 합니다.
3. **확장:** 처음에는 '탄 걸림' 종류를 하나(Dud: 불발)만 구현하고, 나중에 배출 불량(Stovepipe) 등을 추가하십시오.

이 명세서는 **"시스템이 어떻게 도는지"**에 대한 청사진입니다. 이대로 AI에게 지시하면, 서로 호환되지 않는 코드를 짤 위험이 사라집니다. 이제 **데이터 에셋 헤더 파일**부터 생성을 요청하시면 됩니다. "위 명세서의 2.2 섹션에 있는 데이터 에셋들을 C++ 헤더로 짜줘"라고 하면 바로 시작할 수 있습니다.