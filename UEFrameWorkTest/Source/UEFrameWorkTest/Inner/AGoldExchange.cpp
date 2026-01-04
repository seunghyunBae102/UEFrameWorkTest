// GHOST IN THE MONITOR - Gold Exchange Actor Implementation
#include "Inner/AGoldExchange.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Subsystem/Subsystem_EconomyManager.h"
#include "GameFramework/GameStateBase.h"

AGoldExchange::AGoldExchange()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root 컴포넌트: Sphere (상호작용 감지)
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	RootComponent = InteractionSphere;
	InteractionSphere->SetSphereRadius(200.0f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//InteractionSphere->SetCollisionResponseToChannels(ECC_WorldStatic);
	//InteractionSphere->SetCollisionResponseToChannel(ECC_WorldStatic);
	// 시각적 표현: Mesh
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGoldExchange::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("AGoldExchange::BeginPlay - [%s] initialized!"), *ExchangeName.ToString());
}

void AGoldExchange::OnInteract_Implementation(AActor* Interactor)
{
	if (!Interactor)
	{
		UE_LOG(LogTemp, Warning, TEXT("AGoldExchange::OnInteract - Invalid interactor!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("AGoldExchange::OnInteract - [%s] interacting with [%s]"),
		*Interactor->GetName(), *ExchangeName.ToString());

	ProcessDeposit(Interactor);
}

bool AGoldExchange::CanInteract_Implementation() const
{
	// 항상 상호작용 가능
	return true;
}

FText AGoldExchange::GetInteractionPrompt_Implementation() const
{
	return FText::FromString(FString::Printf(TEXT("Press [E] to deposit gold (%.0f)"), DepositAmount));
}

void AGoldExchange::ProcessDeposit(AActor* Player)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("AGoldExchange::ProcessDeposit - No GameInstance!"));
		return;
	}

	USubsystem_EconomyManager* EconomyManager = GameInstance->GetSubsystem<USubsystem_EconomyManager>();
	if (!EconomyManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("AGoldExchange::ProcessDeposit - No EconomyManager subsystem!"));
		return;
	}

	// 입금 처리
	EconomyManager->DepositFunds(DepositAmount);

	// 이벤트 발생
	OnDepositComplete.Broadcast(DepositAmount, EconomyManager->GetCurrentBalance());

	UE_LOG(LogTemp, Log, TEXT("AGoldExchange::ProcessDeposit - Deposited %.0f. New balance: %.2f"),
		DepositAmount, EconomyManager->GetCurrentBalance());
}
