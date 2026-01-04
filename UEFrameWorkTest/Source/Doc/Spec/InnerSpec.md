[TechSpec] Inner World System (The Knight)

1. Character Class: ABP_InnerKnight

상속: ACharacter

인터페이스: BPI_CombatEntity

역할: 컴포넌트 컨테이너 및 입력 라우팅.

2. Component: UBPC_TankMovement

상속: UActorComponent

역할: 가속/감속이 있는 탱크 컨트롤 이동 로직.

2.1. Public Variables

float MaxMoveSpeed: 최대 이동 속도 (Default: 400).

float TurnRate: 회전 속도 (Default: 180 deg/s).

float BackwardsPenalty: 후진 속도 배율 (Default: 0.6).

float Acceleration: 가속도 계수 (RInterpSpeed).

2.2. Key Methods

void Input_Move(float AxisValue):

AddMovementInput을 사용하되, AxisValue를 FMath::FInterpTo로 보간하여 가속감 구현.

후진(Axis < 0) 시 BackwardsPenalty 곱연산.

void Input_Rotate(float AxisValue):

SetActorRotation 사용. 제자리 회전 로직.

void PerformQuickTurn():

입력: Q Key.

로직: 180도 회전 몽타주 재생. 몽타주 종료(AnimNotify) 시점에 실제 액터 Rotation 반전.

3. Component: UBPC_InnerCombat

상속: UActorComponent

역할: 전투 상태 머신, 패링(Clash), 강인도 관리.

3.1. Public Variables

float MaxPoise: 최대 강인도.

float CurrentPoise: 현재 강인도.

bool bIsGuardActive: 가드 상태 플래그.

float ClashWindowTime: 공격 시작 후 패링 인정 시간 (0.2s).

3.2. Key Methods

void AttemptAttack(bool bIsHeavy):

로직: 공격 몽타주 재생. FTimerHandle을 통해 ClashWindowTime 동안 bCanClash = true 설정.

void CheckClash(AActor* OtherWeapon):

로직: 무기 콜리전 오버랩 이벤트에서 호출.

if (bCanClash): 데미지 무효화, 스파크 이펙트 스폰, 적에게 OnStaggered 호출.

void ToggleGuard(bool bEnable):

로직: bIsGuardActive 설정. 이동 속도 50% 감소 처리.

4. Component: UBPC_QuickSlot

상속: UActorComponent

역할: 1~5번 슬롯 아이템 데이터 관리.

4.1. Variables

TArray<UDA_InnerItem*> Slots: 크기 5 고정 배열.

int32 CurrentSlotIndex: 현재 장착 슬롯.

4.2. Methods

void UseSlot(int32 Index):

로직: Index에 해당하는 아이템 타입(무기/소모품)에 따라 EquipWeapon 혹은 ConsumeItem 호출.