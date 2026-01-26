언리얼 엔진 5(UE5) 환경에서 LLM(AI)과 협업하며, God Object를 방지하고, 개발 속도와
유지보수성을 모두 잡기 위한 [Pragmatic Component-Based Architecture (실용적 컴포넌트
기반 아키텍처)] 명세서입니다.
이 문서는 개발 팀원(사람)과 AI(LLM)가 공통으로 따라야 할 **헌법(Constitution)**과 같습니다.

📘 UE5 Pragmatic Clean Architecture
Guide
"Actor is a Container, Component is the Logic."
1. 핵심 철학 (Core Philosophy)
이 아키텍처의 목표는 결합도(Coupling)를 극도로 낮추고, LLM이 작은 단위의 코드를
완벽하게 작성할 수 있도록 돕는 것입니다.
1. Actor as Container (액터는 그릇이다): BP_Character는 Mesh, Collision, Component를
담는 '껍데기'일 뿐입니다. 어떠한 게임 로직(노드)도 액터 블루프린트에 작성하지
않습니다.
2. Logic in Components (로직은 컴포넌트에): 모든 기능(이동, 체력, 공격, 인벤토리)은
UActorComponent 단위로 캡슐화합니다.
3. Data-Driven (데이터 주도): 스탯(체력, 공격력 등)은 변수가 아니라 DataAsset으로
관리합니다.
4. No Direct Casting (직접 참조 금지): Cast To 노드 사용을 엄격히 제한합니다. 대신
Interface와 Event Dispatcher를 사용합니다.

2. 아키텍처 계층 구조 (Architectural Layers)
LLM에게 코드를 요청할 때, 어느 레이어에 해당하는지 명확히 지정해야 합니다.
Layer 1: Data Layer (데이터 정의)
● 형식: C++ UPrimaryDataAsset
● 역할: 기획 데이터 저장소. 로직 없음.
● 규칙: BlueprintReadWrite, EditAnywhere 속성 사용.
● 예시: DA_MonsterStats (HP, MP, DropTable)
Layer 2: Logic Layer (기능 구현)
● 형식: C++ UActorComponent

● 역할: 실제 게임 로직 수행.
● 규칙:
○ Tick 비활성화: 기본적으로 PrimaryComponentTick.bCanEverTick = false. 타이머나
이벤트로 구동.
○ 상태 관리: bool 대신 GameplayTags 사용 권장.
● 예시: BPC_Health, BPC_Combat
Layer 3: Communication Layer (통신)
● 형식: Blueprint Interface (BPI) & Event Dispatcher
● 역할: 컴포넌트 간, 액터 간 연결.
● 규칙: 액터 참조(Reference)를 변수로 저장하지 않음.
Layer 4: Assembly Layer (조립)
● 형식: Blueprint Actor (BP_Player, BP_Goblin)
● 역할: 위 1, 2, 3을 조립하고 시각적 효과(VFX/SFX)만 연결.
● 규칙: "No Logic Nodes Allowed" (Event Graph가 비어있거나, 초기화 호출만 존재해야
함).

3. 통신 프로토콜 (Communication Protocol)
God Object를 막는 핵심은 **"어떻게 대화하느냐"**입니다.
상황 금지된 방식 (Bad) 권장 방식 (Clean)
피격 판정 Cast To Character ->
HP_Variable -= 10

GetComponentByInterface(
Damageable) ->
TakeDamage()

죽음 처리 캐릭터가 HUD 위젯을 찾아
ShowGameOver 호출

HealthComponent가
OnDeath Dispatch, HUD가
이를 Listen

입력 처리 캐릭터 BP에서 Input Action

-> 로직 수행

Input Action ->
CombatComponent의 함수
호출
상태 체크 if (bIsStunned == true) if

(Tags.HasTag("State.Debuf
f.Stun"))

4. 단계별 구현 워크플로우 (Step-by-Step Workflow)
사람과 AI가 협업하는 표준 절차입니다.
[Step 1] 데이터 설계 (Data Asset)
● Human: "몬스터 스탯이 필요해."
● AI Prompt: "RPG 몬스터 스탯을 정의하는 UPrimaryDataAsset 기반 C++ 클래스
DA_MonsterStats를 작성해줘. MaxHealth, AttackPower, WalkSpeed를 포함해."
[Step 2] 로직 컴포넌트 제작 (C++ Component)
● Human: "체력 관리 기능이 필요해."
● AI Prompt:

"UActorComponent를 상속받는 HealthComponent를 작성해.
1. DA_MonsterStats를 변수로 받아서 BeginPlay에서 초기화해.
2. TakeDamage(float Amount) 함수를 구현해.
3. 체력이 변할 때와 0이 될 때를 알리는 Event Dispatcher를 선언해.
4. Tick은 꺼줘."

[Step 3] 인터페이스 정의 (Interface)
● Human: "외부에서 때릴 수 있어야 해."
● AI Prompt: "BPI_Damageable 인터페이스를 만들고 TakeDamage 함수를 정의해줘.
HealthComponent가 이 인터페이스를 구현하도록 해." (※ C++ 혹은 BP에서 구현)
[Step 4] 블루프린트 조립 (Editor)
● Human: (에디터 작업)
1. BP_Goblin 생성.
2. HealthComponent 추가.
3. DataAsset 슬롯에 데이터 할당.
4. 끝. (이벤트 그래프에 노드 짜지 않음)

5. AI(LLM) 시스템 프롬프트 (복사해서 사용)
새로운 대화창을 열 때마다 이 프롬프트를 먼저 입력하여 AI를 '시니어 언리얼 개발자'로
세팅하세요.

Markdown

You are a Senior Unreal Engine 5 Developer specialized in Modular & Data-Driven
Architecture.
Follow these strict guidelines to prevent "God Objects" and ensure clean code:
1. [Actor as Container]: Never write logic inside `ACharacter` or `AActor` classes. Use them only
to hold Components and Data Assets.
2. [Component-Based]: All features (Movement, Stats, Combat) must be implemented as
`UActorComponent`.
3. [No Direct Casting]: Never use `Cast<AMyCharacter>`. Use `Interface` calls or
`FindComponentByClass` to interact.
4. [Event-Driven]: Use `Delegates` (Event Dispatchers) for outputs. Do not let components
reference UI or GameMode directly.
5. [Data Assets]: Use `UPrimaryDataAsset` for static data (Stats), not hardcoded variables.
6. [Gameplay Tags]: Use Gameplay Tags for state management (e.g., "State.Dead") instead of
boolean variables.
If I ask for a feature (e.g., "Attack System"), provide the `Component` C++ code and the
`Interface` definition, NOT the Character code.

6. 검증 체크리스트 (Verification)
개발 중간에 다음 항목을 체크하여 '스파게티화'를 방지하십시오.
● [ ] 액터 블루프린트가 깨끗한가? (노드가 거의 없어야 함)
● [ ] Cast To 노드가 없는가? (인터페이스/컴포넌트 찾기로 대체)
● [ ] 컴포넌트의 Tick이 꺼져 있는가? (필요한 경우만 켬)
● [ ] UI가 캐릭터를 직접 참조하지 않는가? (이벤트 바인딩 사용)
● [ ] bool 변수 대신 Gameplay Tag를 쓰고 있는가?

언리얼 시니어의 조언 (Conclusion)
이 가이드는 **"유니티의 직관성(컴포넌트 조립)"**과 **"언리얼의 강력함(데이터 에셋, 태그,
디스패처)"**의 교집합입니다.
처음에는 컴포넌트와 데이터 에셋을 만드는 게 번거로워 보일 수 있습니다. 하지만 기능이
3개만 넘어가도, 액터 하나에 모든 걸 때려 박은 프로젝트보다 개발 속도가 압도적으로
빨라집니다. AI에게 작업을 시킬 때도 "이 컴포넌트만 고쳐줘"라고 할 수 있어 정확도가 매우
높아집니다.

이제 이 명세서를 바탕으로 AI에게 첫 번째 컴포넌트를 주문해 보십시오.