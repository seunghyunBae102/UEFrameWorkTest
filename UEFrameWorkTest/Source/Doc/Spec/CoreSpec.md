[TechSpec] Core Types & Shared Data

1. 개요

프로젝트 전반에서 공통으로 사용되는 Enum, Struct, Interface 정의입니다.

2. Enums (Enumerations)

2.1. EInnerState (Inner World 상태)

용도: 기사의 애니메이션 및 입력 허용 판정.

값:

Idle: 대기

Locomotion: 이동 중 (가드 가능)

Attacking: 공격 중 (이동/회전 불가)

Guarding: 방어 중 (이동 속도 감소)

Dodging: 회피 중 (무적)

Staggered: 그로기 (조작 불가)

Dead: 사망

2.2. EWeaponPart (총기 부품)

용도: Outer World 총기 상호작용 식별.

값:

Magazine: 탄창 본체

MagWell: 빈 삽탄구

Bolt: 장전 손잡이/노리쇠

Selector: 조정간

None: 해당 없음

2.3. EOuterState (플레이어 상태)

용도: 입력 컨텍스트(IMC) 전환.

값:

Sitting: 책상 앞 (Inner 조작 가능)

Roaming: 방 안 자유 이동 (WASD)

Inspection: 'I' 키 홀드 중 (공간적 UI)

StationLocked: 특정 사물(문, PC) 상호작용 중

3. Structs (Structures)

3.1. FInnerDamageEvent

용도: Inner World 피격 판정 데이터 전송.

필드:

float Amount: HP 데미지 (public)

float PoiseDamage: 강인도 데미지 (public)

FVector HitDirection: 피격 방향 (넉백용) (public)

bool bIsMetaAttack: 현실 총기에 의한 공격 여부 (Friendly Fire 판정용) (public)

4. Interfaces

4.1. BPI_CombatEntity

용도: Inner World의 모든 캐릭터(기사, 몬스터)가 상속.

메서드:

void ReceiveDamage(FInnerDamageEvent Event): 데미지 처리.

float GetCurrentPoise(): 현재 강인도 반환.

void OnMetaImpact(FVector ImpactPoint, float Force): 포탈 사격 피격 처리.

4.2. BPI_Interactable

용도: Outer World 상호작용 (문, PC, 총기 부품).

메서드:

void OnInteract(APlayerController* PC): E키 상호작용.

void OnHoverStart(): 마우스 오버 시 하이라이트.

void OnHoverEnd(): 하이라이트 해제.