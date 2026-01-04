// GHOST IN THE MONITOR - Economy Manager Subsystem
#include "Subsystem/Subsystem_EconomyManager.h"

void USubsystem_EconomyManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentInflationIndex = 1.0f;
	GlobalRiskLevel = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Subsystem_EconomyManager::Initialize - Economy Manager initialized!"));
}

void USubsystem_EconomyManager::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("Subsystem_EconomyManager::Deinitialize - Economy Manager shut down!"));
	Super::Deinitialize();
}

float USubsystem_EconomyManager::GetDynamicPrice(const FName& ItemID)
{
	const FMarketItem* Item = FindMarketItem(ItemID);
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("Subsystem_EconomyManager::GetDynamicPrice - Item [%s] not found!"), *ItemID.ToString());
		return 0.0f;
	}

	// 공식: FinalPrice = BasePrice * Scarcity * CurrentInflationIndex * (1 + GlobalRiskLevel * 0.1)
	float RiskMultiplier = 1.0f + (GlobalRiskLevel * 0.1f);
	float FinalPrice = Item->BasePrice * Item->Scarcity * CurrentInflationIndex * RiskMultiplier;

	// 최소 가격 적용
	FinalPrice = FMath::Max(Item->BasePrice * MinPriceMultiplier, FinalPrice);

	return FinalPrice;
}

void USubsystem_EconomyManager::AdvanceInflation(float IncreaseAmount)
{
	CurrentInflationIndex += IncreaseAmount;
	CurrentInflationIndex = FMath::Min(CurrentInflationIndex, MaxInflationIndex);

	UE_LOG(LogTemp, Log, TEXT("Subsystem_EconomyManager::AdvanceInflation - New inflation index: %.2f"), CurrentInflationIndex);

	UpdateAllPrices();
	OnEconomyChanged.Broadcast(CurrentInflationIndex, GlobalRiskLevel);
}

void USubsystem_EconomyManager::IncreaseGlobalRisk(float RiskAmount)
{
	GlobalRiskLevel += RiskAmount;
	GlobalRiskLevel = FMath::Max(0.0f, GlobalRiskLevel);

	UE_LOG(LogTemp, Log, TEXT("Subsystem_EconomyManager::IncreaseGlobalRisk - Global risk level: %.2f"), GlobalRiskLevel);

	UpdateAllPrices();
	OnEconomyChanged.Broadcast(CurrentInflationIndex, GlobalRiskLevel);
}

void USubsystem_EconomyManager::DecreaseGlobalRisk(float RecoveryAmount)
{
	GlobalRiskLevel -= RecoveryAmount;
	GlobalRiskLevel = FMath::Max(0.0f, GlobalRiskLevel);

	UE_LOG(LogTemp, Log, TEXT("Subsystem_EconomyManager::DecreaseGlobalRisk - Global risk level: %.2f"), GlobalRiskLevel);

	UpdateAllPrices();
	OnEconomyChanged.Broadcast(CurrentInflationIndex, GlobalRiskLevel);
}

void USubsystem_EconomyManager::ModifyStock(const FName& ItemID, int32 QuantityChange)
{
	FMarketItem* Item = FindMarketItem(ItemID);
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("Subsystem_EconomyManager::ModifyStock - Item [%s] not found!"), *ItemID.ToString());
		return;
	}

	Item->StockQuantity += QuantityChange;
	Item->StockQuantity = FMath::Max(0, Item->StockQuantity);

	UE_LOG(LogTemp, Log, TEXT("Subsystem_EconomyManager::ModifyStock - Item [%s] stock: %d"), *ItemID.ToString(), Item->StockQuantity);

	float NewPrice = GetDynamicPrice(ItemID);
	OnPriceChanged.Broadcast(ItemID, NewPrice, Item->Scarcity);
}

int32 USubsystem_EconomyManager::GetStock(const FName& ItemID) const
{
	const FMarketItem* Item = FindMarketItem(ItemID);
	return Item ? Item->StockQuantity : 0;
}

float USubsystem_EconomyManager::GetScarcity(const FName& ItemID) const
{
	const FMarketItem* Item = FindMarketItem(ItemID);
	return Item ? Item->Scarcity : 1.0f;
}

void USubsystem_EconomyManager::ResetEconomy()
{
	CurrentInflationIndex = 1.0f;
	GlobalRiskLevel = 0.0f;
	CurrentAccountBalance = InitialBalance;

	// 모든 아이템 재고 리셋
	for (FMarketItem& Item : MarketItems)
	{
		Item.StockQuantity = 100;  // 기본값으로 리셋
	}

	UE_LOG(LogTemp, Log, TEXT("Subsystem_EconomyManager::ResetEconomy - Economy and wallet reset! Balance: %.2f"), CurrentAccountBalance);

	OnEconomyChanged.Broadcast(CurrentInflationIndex, GlobalRiskLevel);
	OnBalanceChanged.Broadcast(CurrentAccountBalance, true);
}

FMarketItem* USubsystem_EconomyManager::FindMarketItem(const FName& ItemID)
{
	for (FMarketItem& Item : MarketItems)
	{
		if (Item.ItemID == ItemID)
		{
			return &Item;
		}
	}
	return nullptr;
}

const FMarketItem* USubsystem_EconomyManager::FindMarketItem(const FName& ItemID) const
{
	for (const FMarketItem& Item : MarketItems)
	{
		if (Item.ItemID == ItemID)
		{
			return &Item;
		}
	}
	return nullptr;
}

void USubsystem_EconomyManager::UpdateAllPrices()
{
	for (const FMarketItem& Item : MarketItems)
	{
		float NewPrice = GetDynamicPrice(Item.ItemID);
		OnPriceChanged.Broadcast(Item.ItemID, NewPrice, Item.Scarcity);
	}
}

void USubsystem_EconomyManager::DepositFunds(float Amount)
{
	if (Amount <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Subsystem_EconomyManager::DepositFunds - Invalid amount: %.2f"), Amount);
		return;
	}

	CurrentAccountBalance += Amount;

	UE_LOG(LogTemp, Log, TEXT("Subsystem_EconomyManager::DepositFunds - Deposited %.2f. New balance: %.2f"),
		Amount, CurrentAccountBalance);

	OnBalanceChanged.Broadcast(CurrentAccountBalance, true);
}

bool USubsystem_EconomyManager::TrySpendFunds(float Amount)
{
	if (Amount <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Subsystem_EconomyManager::TrySpendFunds - Invalid amount: %.2f"), Amount);
		return false;
	}

	if (CurrentAccountBalance < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("Subsystem_EconomyManager::TrySpendFunds - Insufficient funds! Required: %.2f, Have: %.2f"),
			Amount, CurrentAccountBalance);

		OnInsufficientFunds.Broadcast(Amount, CurrentAccountBalance);
		return false;
	}

	CurrentAccountBalance -= Amount;

	UE_LOG(LogTemp, Log, TEXT("Subsystem_EconomyManager::TrySpendFunds - Spent %.2f. Remaining balance: %.2f"),
		Amount, CurrentAccountBalance);

	OnBalanceChanged.Broadcast(CurrentAccountBalance, false);
	return true;
}

void USubsystem_EconomyManager::SetBalance(float NewBalance)
{
	CurrentAccountBalance = FMath::Max(0.0f, NewBalance);

	UE_LOG(LogTemp, Log, TEXT("Subsystem_EconomyManager::SetBalance - Balance set to: %.2f"), CurrentAccountBalance);

	OnBalanceChanged.Broadcast(CurrentAccountBalance, NewBalance > CurrentAccountBalance);
}
