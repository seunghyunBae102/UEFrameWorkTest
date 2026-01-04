[TechSpec] Meta & Economy System

1. Component: UBPC_PortalMechanic

상속: UActorComponent (모니터 액터에 부착)

역할: Outer 사격을 Inner 물리력으로 변환.

1.1. Key Methods

void ProcessMonitorHit(FHitResult OuterHit, float ImpactForce):

입력: Outer World 총알의 히트 정보.

로직:

UGameplayStatics::FindCollisionUV로 히트 UV 추출.

ConvertUVToInnerLocation(UV) 실행.

SpawnInnerImpact 실행.

FVector ConvertUVToInnerLocation(FVector2D UV):

로직: InnerWorldOrigin + (RightVector * UV.X * MapWidth) + (UpVector * UV.Y * MapHeight).

void SpawnInnerImpact(FVector Location, float Force):

로직: 해당 위치에 RadialForceActor 스폰. FireImpulse() 호출하여 범위 내 BPI_CombatEntity들에게 충격 전달.

2. Actor Class: AGoldExchange (Inner World)

상속: AActor

인터페이스: BPI_Interactable (Inner용)

역할: 골드 환전 오브젝트.

2.1. Methods

void OnInteract(ACharacter* Interactor):

로직: Interactor의 소지 골드를 0으로 만듦.

델리게이트 호출: OnGoldExchanged.Broadcast(Amount).

이 델리게이트는 GameInstance 혹은 PC_MetaController가 바인딩하여 현실 자금($)을 증가시킴.

3. Actor Class: ATerminal (Outer World)

상속: AActor (PC 본체/모니터)

인터페이스: BPI_Interactable (Outer용)

역할: 탄약 상점.

3.1. Methods

void OnInteract(APlayerController* PC):

로직: PC 화면 UI 위젯 (WBP_Shop) 활성화.

void OrderItem(UDA_ShopItem* Item):

로직: 자금($) 차감. RoomEventManager에 배송 타이머 설정.