// GHOST IN THE MONITOR - Magazine Actor Base Implementation
#include "Outer/AMagazineBase.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Outer/BPC_MagLogistic.h"
#include "Data/DA_AmmoType.h"
#include "GameFramework/Actor.h"

AMagazineBase::AMagazineBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.TickInterval = 0.0f;

	// 루트 컴포넌트 설정
	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComp;

	// 메시 컴포넌트 설정
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 물리 충돌 컴포넌트 설정
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	CollisionComponent->SetupAttachment(RootComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// 탄약 관리 컴포넌트 설정
	MagLogisticComponent = CreateDefaultSubobject<UBP_MagLogistic>(TEXT("MagLogistic"));
	MagLogisticComponent->SetupAttachment(RootComponent);

	// 기본값 설정
	MaxCapacity = 30;
	MagazineType = TEXT("9x19mm");
	MagazineID = TEXT("MAG_Unknown");
	bIsInInventory = false;
	bIsSimulatingPhysics = false;
	AttachedToWeapon = nullptr;

	// 물리 설정
	if (CollisionComponent)
	{
		CollisionComponent->BodyInstance.bSimulatePhysics = false;
		CollisionComponent->SetSimulatePhysics(false);
	}
}

void AMagazineBase::BeginPlay()
{
	Super::BeginPlay();

	// 컴포넌트 검증
	if (!MagLogisticComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AMagazineBase::BeginPlay [%s] - MagLogisticComponent is null!"), *GetName());
		return;
	}

	// 기본 용량 설정
	MagLogisticComponent->Capacity = MaxCapacity;

	// 이벤트 바인딩
	BindMagLogisticEvents();

	UE_LOG(LogTemp, Log, TEXT("AMagazineBase::BeginPlay [%s] - Magazine initialized. Type: %s, Capacity: %d"),
		*GetName(), *MagazineType.ToString(), MaxCapacity);
}

void AMagazineBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMagazineBase::InitializeMagazine(UDA_AmmoType* AmmoType, int32 AmmoCount)
{
	if (!AmmoType)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMagazineBase::InitializeMagazine [%s] - Invalid ammo type!"), *GetName());
		return;
	}

	if (!MagLogisticComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AMagazineBase::InitializeMagazine [%s] - MagLogisticComponent is null!"), *GetName());
		return;
	}

	// 탄약 추가
	int32 LoadedCount = 0;
	for (int32 i = 0; i < AmmoCount && LoadedCount < MaxCapacity; ++i)
	{
		float InsertTime = 0.0f;
		if (MagLogisticComponent->InsertBullet(AmmoType, InsertTime))
		{
			LoadedCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AMagazineBase::InitializeMagazine [%s] - Loaded %d / %d rounds of %s"),
		*GetName(), LoadedCount, AmmoCount, *AmmoType->CaliberName.ToString());

	UpdateVisuals();
}

bool AMagazineBase::AttachToWeapon(AActor* WeaponActor, const FName& SocketName)
{
	if (!WeaponActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMagazineBase::AttachToWeapon [%s] - Invalid weapon actor!"), *GetName());
		return false;
	}

	// 이미 부착된 경우
	if (AttachedToWeapon != nullptr && AttachedToWeapon != WeaponActor)
	{
		DetachFromWeapon();
	}

	// 소켓에 부착
	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
	AttachToActor(WeaponActor, AttachRules, SocketName);

	AttachedToWeapon = WeaponActor;
	bIsInInventory = false;
	bIsSimulatingPhysics = false;

	// 물리 비활성화
	if (CollisionComponent)
	{
		CollisionComponent->SetSimulatePhysics(false);
	}

	// 메시 표시
	MeshComponent->SetVisibility(true);

	OnMagazineAttached.Broadcast(WeaponActor);

	UE_LOG(LogTemp, Log, TEXT("AMagazineBase::AttachToWeapon [%s] - Attached to weapon [%s] at socket [%s]"),
		*GetName(), *WeaponActor->GetName(), *SocketName.ToString());

	return true;
}

bool AMagazineBase::DetachFromWeapon()
{
	if (AttachedToWeapon == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMagazineBase::DetachFromWeapon [%s] - Not attached to any weapon!"), *GetName());
		return false;
	}

	// 분리 규칙 설정
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	DetachFromActor(DetachRules);

	AttachedToWeapon = nullptr;
	bIsInInventory = false;

	OnMagazineDetached.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("AMagazineBase::DetachFromWeapon [%s] - Detached from weapon"),
		*GetName());

	return true;
}

void AMagazineBase::TakeToInventory()
{
	if (bIsInInventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMagazineBase::TakeToInventory [%s] - Already in inventory!"), *GetName());
		return;
	}

	// 무기에 부착되어 있으면 분리
	if (AttachedToWeapon != nullptr)
	{
		DetachFromWeapon();
	}

	bIsInInventory = true;
	bIsSimulatingPhysics = false;

	// 메시 숨기기
	MeshComponent->SetVisibility(false);

	// 물리 비활성화
	if (CollisionComponent)
	{
		CollisionComponent->SetSimulatePhysics(false);
		CollisionComponent->SetCollisionEnabled(ECC_NoCollision);
	}

	OnTakenToInventory.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("AMagazineBase::TakeToInventory [%s] - Taken to inventory"), *GetName());
}

void AMagazineBase::DropFromInventory(FVector DropLocation, FVector DropVelocity)
{
	if (!bIsInInventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMagazineBase::DropFromInventory [%s] - Not in inventory!"), *GetName());
		return;
	}

	// 위치 설정
	SetActorLocation(DropLocation);

	bIsInInventory = false;
	bIsSimulatingPhysics = true;

	// 메시 표시
	MeshComponent->SetVisibility(true);

	// 물리 활성화
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECC_QueryAndPhysics);
		CollisionComponent->SetSimulatePhysics(true);

		// 초기 속도 설정
		if (DropVelocity.Length() > 0.0f)
		{
			CollisionComponent->SetPhysicsLinearVelocity(DropVelocity);
		}
	}

	OnDroppedFromInventory.Broadcast(DropLocation);

	UE_LOG(LogTemp, Log, TEXT("AMagazineBase::DropFromInventory [%s] - Dropped at [%.2f, %.2f, %.2f]"),
		*GetName(), DropLocation.X, DropLocation.Y, DropLocation.Z);
}

int32 AMagazineBase::GetAmmoCount() const
{
	if (!MagLogisticComponent)
	{
		return 0;
	}
	return MagLogisticComponent->GetAmmoCount();
}

float AMagazineBase::GetCurrentWeight() const
{
	if (!MagLogisticComponent)
	{
		return 0.0f;
	}
	return MagLogisticComponent->GetCurrentWeight();
}

bool AMagazineBase::IsEmpty() const
{
	if (!MagLogisticComponent)
	{
		return true;
	}
	return MagLogisticComponent->IsEmpty();
}

bool AMagazineBase::IsFull() const
{
	if (!MagLogisticComponent)
	{
		return false;
	}
	return MagLogisticComponent->IsFull();
}

int32 AMagazineBase::AddAmmo(UDA_AmmoType* Ammo, int32 Count)
{
	if (!Ammo || !MagLogisticComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMagazineBase::AddAmmo [%s] - Invalid ammo or component!"), *GetName());
		return 0;
	}

	int32 LoadedCount = 0;
	for (int32 i = 0; i < Count; ++i)
	{
		float InsertTime = 0.0f;
		if (MagLogisticComponent->InsertBullet(Ammo, InsertTime))
		{
			LoadedCount++;
		}
		else
		{
			break;  // 탄창이 가득 찼으므로 중단
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AMagazineBase::AddAmmo [%s] - Added %d ammo. Total: %d"),
		*GetName(), LoadedCount, GetAmmoCount());

	UpdateVisuals();
	return LoadedCount;
}

bool AMagazineBase::ConsumeAmmo(UDA_AmmoType*& OutAmmo)
{
	if (!MagLogisticComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMagazineBase::ConsumeAmmo [%s] - MagLogisticComponent is null!"), *GetName());
		return false;
	}

	if (!MagLogisticComponent->RemoveBullet(OutAmmo))
	{
		UE_LOG(LogTemp, Warning, TEXT("AMagazineBase::ConsumeAmmo [%s] - Magazine is empty!"), *GetName());
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("AMagazineBase::ConsumeAmmo [%s] - Fired. Remaining: %d"),
		*GetName(), GetAmmoCount());

	UpdateVisuals();
	return true;
}

void AMagazineBase::EmptyMagazine()
{
	if (!MagLogisticComponent)
	{
		return;
	}

	MagLogisticComponent->ClearMagazine();

	UE_LOG(LogTemp, Log, TEXT("AMagazineBase::EmptyMagazine [%s] - Magazine emptied"), *GetName());

	UpdateVisuals();
}

EMagazineWeightStatus AMagazineBase::GetWeightStatus() const
{
	if (!MagLogisticComponent)
	{
		return EMagazineWeightStatus::Empty;
	}
	return MagLogisticComponent->EstimateWeightStatus();
}

void AMagazineBase::SetPhysicsSimulation(bool bEnable)
{
	if (!CollisionComponent)
	{
		return;
	}

	bIsSimulatingPhysics = bEnable;
	CollisionComponent->SetSimulatePhysics(bEnable);

	UE_LOG(LogTemp, Log, TEXT("AMagazineBase::SetPhysicsSimulation [%s] - Physics simulation: %s"),
		*GetName(), bEnable ? TEXT("ON") : TEXT("OFF"));
}

void AMagazineBase::UpdateVisuals()
{
	if (!MeshComponent)
	{
		return;
	}

	// 탄약 개수에 따라 무게 상태 업데이트
	// 향후: 메시 교체, 텍스처 업데이트 등 구현 가능
	EMagazineWeightStatus WeightStatus = GetWeightStatus();

	switch (WeightStatus)
	{
	case EMagazineWeightStatus::Empty:
		UE_LOG(LogTemp, Log, TEXT("AMagazineBase::UpdateVisuals [%s] - Magazine is EMPTY"), *GetName());
		break;
	case EMagazineWeightStatus::Light:
		UE_LOG(LogTemp, Log, TEXT("AMagazineBase::UpdateVisuals [%s] - Magazine is LIGHT"), *GetName());
		break;
	case EMagazineWeightStatus::Medium:
		UE_LOG(LogTemp, Log, TEXT("AMagazineBase::UpdateVisuals [%s] - Magazine is MEDIUM"), *GetName());
		break;
	case EMagazineWeightStatus::Heavy:
		UE_LOG(LogTemp, Log, TEXT("AMagazineBase::UpdateVisuals [%s] - Magazine is HEAVY"), *GetName());
		break;
	case EMagazineWeightStatus::Full:
		UE_LOG(LogTemp, Log, TEXT("AMagazineBase::UpdateVisuals [%s] - Magazine is FULL"), *GetName());
		break;
	}
}

void AMagazineBase::BindMagLogisticEvents()
{
	if (!MagLogisticComponent)
	{
		return;
	}

	// MagLogistic의 이벤트를 자신의 이벤트로 재전파
	MagLogisticComponent->OnBulletInserted.AddDynamic(this, &AMagazineBase::OnMagLogisticBulletInserted);
	MagLogisticComponent->OnBulletRemoved.AddDynamic(this, &AMagazineBase::OnMagLogisticBulletRemoved);

	UE_LOG(LogTemp, Log, TEXT("AMagazineBase::BindMagLogisticEvents [%s] - Events bound"), *GetName());
}

void AMagazineBase::OnMagLogisticBulletInserted(UDA_AmmoType* InsertedAmmo, int32 CurrentCount)
{
	if (!ensure(InsertedAmmo))
	{
		return;
	}

	float CurrentWeight = GetCurrentWeight();
	OnMagazineBulletAdded.Broadcast(CurrentCount, CurrentWeight);

	UE_LOG(LogTemp, Log, TEXT("AMagazineBase::OnMagLogisticBulletInserted [%s] - Bullet added. Count: %d, Weight: %.2f"),
		*GetName(), CurrentCount, CurrentWeight);
}

void AMagazineBase::OnMagLogisticBulletRemoved(UDA_AmmoType* RemovedAmmo, int32 CurrentCount)
{
	if (!ensure(RemovedAmmo))
	{
		return;
	}

	float CurrentWeight = GetCurrentWeight();
	OnMagazineBulletRemoved.Broadcast(CurrentCount, CurrentWeight);

	UE_LOG(LogTemp, Log, TEXT("AMagazineBase::OnMagLogisticBulletRemoved [%s] - Bullet removed. Count: %d, Weight: %.2f"),
		*GetName(), CurrentCount, CurrentWeight);
}
