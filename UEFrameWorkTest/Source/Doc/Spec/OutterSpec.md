[TechSpec] Outer World System (Logistics)

1. Controller Class: APC_MetaController

상속: APlayerController

역할: 입력 컨텍스트(IMC) 관리, 공간적 UI(I키) 제어.

1.1. Key Methods

void SetInspectionMode(bool bEnable):

입력: I Key (Hold/Release).

로직:

bEnable == true: bShowMouseCursor = true, SetInputMode(GameAndUI). 카메라 FOV 110으로 변경.

bEnable == false: bShowMouseCursor = false, SetInputMode(GameOnly). 카메라 복구.

void UpdateSpatialCamera(float DeltaTime):

로직: PlayerTick 내부에서 호출. 마우스 위치(Viewport 0~1)에 따라 ControlRotation에 오프셋 적용 (Deadzone 로직).

2. Actor Class: AWeaponBase

상속: AActor

역할: 총기 로직, 부품별 히트박스 관리, 사격.

2.1. Components

USkeletalMeshComponent* Mesh: 총기 모델.

UBoxComponent* Box_Mag: 탄창 상호작용 콜리전.

UBoxComponent* Box_Bolt: 노리쇠 상호작용 콜리전.

UBoxComponent* Box_Selector: 조정간 상호작용 콜리전.

2.2. Variables

AMagazineBase* CurrentMag: 현재 장착된 탄창 액터.

bool bIsChambered: 약실 탄 유무.

EFireMode CurrentFireMode: 단발/연사.

2.3. Key Methods

void InteractWithPart(EWeaponPart Part):

Part == Magazine: EjectMag() 호출.

Part == MagWell: InventoryComp->GetSelectedMag() 확인 후 InsertMag() 호출.

Part == Bolt: PullBolt() 호출 (애니메이션 + 잼 제거 + 약실 탄 배출).

void PerformAutoSwap(AMagazineBase* NewMag):

로직: EjectMag()(기존 탄창 인벤토리 이동) -> InsertMag(NewMag) 시퀀스 실행.

void Fire():

로직: CurrentMag에서 탄 차감 실패 시 격발 불가. 성공 시 LineTrace 실행 후 PortalMechanic 호출.

3. Actor Class: AMagazineBase

상속: AActor

역할: 탄알 데이터 보존 및 시각화.

3.1. Variables

TArray<UDA_AmmoType*> BulletStack: LIFO 스택 구조.

int32 MaxCapacity: 최대 장탄수.

3.2. Key Methods

void AddBullet(UDA_AmmoType* Ammo): 스택에 Push. 꽉 차면 리턴 False.

UDA_AmmoType* ConsumeBullet(): 스택 Pop 및 반환.

void UpdateVisuals():

로직: BulletStack을 순회하며 1xN 텍스처 생성. MaterialInstanceDynamic에 텍스처 파라미터 전달 (Glock Style Indicator).

4. Component: UBPC_InventoryLayout

상속: USceneComponent (책상 액터에 부착)

역할: 책상 위 아이템 렌더링 (No Physics).

4.1. Methods

void RefreshLayout(const TArray<FInventoryItem>& Items):

로직: 아이템 데이터에 따라 UStaticMeshComponent들을 소켓 위치에 생성/갱신.

FInventoryItem GetItemAtHit(FHitResult Hit):

로직: 마우스 Trace 결과에 해당하는 아이템 데이터 반환.