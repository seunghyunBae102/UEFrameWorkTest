// GHOST IN THE MONITOR - Terminal Actor Implementation
#include "Outer/ATerminal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Data/DA_ShopItem.h"
#include "Subsystem/Subsystem_EconomyManager.h"
#include "TimerManager.h"
#include "Engine/World.h"

ATerminal::ATerminal()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root 컴포넌트: Sphere (상호작용 감지)
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	RootComponent = InteractionSphere;
	InteractionSphere->SetSphereRadius(200.0f);
	//InteractionSphere->SetCollisionEnabled(ECC_WorldStatic);

	// 시각적 표현: Mesh
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATerminal::BeginPlay()
{
	Super::BeginPlay();

	bShopOpen = false;

	UE_LOG(LogTemp, Log, TEXT("ATerminal::BeginPlay - [%s] initialized! Available items: %d"),
		*TerminalName.ToString(), AvailableItems.Num());
}

void ATerminal::OnInteract_Implementation(AActor* Interactor)
{
	if (!Interactor)
	{
		UE_LOG(LogTemp, Warning, TEXT("ATerminal::OnInteract - Invalid interactor!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ATerminal::OnInteract - [%s] using [%s]"),
		*Interactor->GetName(), *TerminalName.ToString());

	// 상점 UI 열기
	if (!bShopOpen)
	{
		bShopOpen = true;
		OpenShopUI();
		OnShopOpened.Broadcast();

		UE_LOG(LogTemp, Log, TEXT("ATerminal::OnInteract - Shop opened!"));
	}
}

bool ATerminal::CanInteract_Implementation() const
{
	// 항상 상호작용 가능 (UI를 통해 구매 거부 처리)
	return true;
}

FText ATerminal::GetInteractionPrompt_Implementation() const
{
	return FText::FromString(FString::Printf(TEXT("Press [E] to access terminal")));
}

bool ATerminal::PurchaseItem(UDA_ShopItem* Item, AActor* Buyer)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("ATerminal::PurchaseItem - Invalid item!"));
		return false;
	}

	if (!Item->bInStock)
	{
		UE_LOG(LogTemp, Warning, TEXT("ATerminal::PurchaseItem - Item [%s] out of stock!"), *Item->DisplayName.ToString());
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("ATerminal::PurchaseItem - No GameInstance!"));
		return false;
	}

	USubsystem_EconomyManager* EconomyManager = GameInstance->GetSubsystem<USubsystem_EconomyManager>();
	if (!EconomyManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ATerminal::PurchaseItem - No EconomyManager subsystem!"));
		return false;
	}

	// 동적 가격 계산
	float FinalPrice = EconomyManager->GetDynamicPrice(Item->Category);

	// 구매 시도 (자금 차감)
	if (!EconomyManager->TrySpendFunds(Item->BasePrice))
	{
		UE_LOG(LogTemp, Warning, TEXT("ATerminal::PurchaseItem - Insufficient funds for [%s]"), *Item->DisplayName.ToString());
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("ATerminal::PurchaseItem - Purchased [%s] for %.2f. Remaining balance: %.2f"),
		*Item->DisplayName.ToString(), Item->BasePrice, EconomyManager->GetCurrentBalance());

	// 배송 시뮬레이션 (배달 시간 후 아이템 도착)
	if (Item->DeliveryTime > 0.0f && GetWorld())
	{
		FTimerDelegate DeliveryDelegate;
		DeliveryDelegate.BindUFunction(this, FName("OnDeliveryComplete"));
		//DeliveryDelegate.BindUFunction(this,&OnDeliveryComplete);
		GetWorld()->GetTimerManager().SetTimer(
			DeliveryTimerHandle,
			DeliveryDelegate,
			Item->DeliveryTime,
			false
		);
	}

	// 이벤트 발생
	OnPurchaseComplete.Broadcast(Item, EconomyManager->GetCurrentBalance());

	return true;
}

void ATerminal::OnDeliveryComplete()
{
	UE_LOG(LogTemp, Log, TEXT("ATerminal::OnDeliveryComplete - Item delivered!"));
	// Blueprint에서 아이템 추가 등의 로직 구현
}
