// GHOST IN THE MONITOR - Economy Manager Subsystem
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystem_EconomyManager.generated.h"

/**
 * 시장 아이템 데이터 (기획자가 수정 가능한 경제 데이터)
 */
USTRUCT(BlueprintType)
struct FMarketItem
{
	GENERATED_BODY()

	// 아이템 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	// 기본 가격 (기준값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasePrice = 100.0f;

	// 희소성 (물품의 부족도 / 수요도)
	// 1.0 = 중립, 1.5 = 50% 더 비쌈
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Scarcity = 1.0f;

	// 현재 재고량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StockQuantity = 100;
};

/**
 * USubsystem_EconomyManager - 게임 전역 경제 시스템
 * 
 * 역할:
 * - 전역적인 물가 관리 (인플레이션)
 * - 상점 재고 관리
 * - 동적 가격 계산
 * - 난이도 상승에 따른 경제 변화
 * 
 * 규칙:
 * - GameInstance 라이프사이클을 따름
 * - 모든 인게임 경제 이벤트는 여기서 처리
 * - 각 월드는 자신의 경제 상태를 별도로 가질 수 있음
 */
UCLASS()
class UEFRAMEWORKTEST_API USubsystem_EconomyManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==================== Properties ====================

	// 현재 인플레이션 지수 (기본 1.0)
	// 스테이지를 깰수록 증가하여 난이도와 경제를 동시에 상향
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
	float CurrentInflationIndex = 1.0f;

	// 글로벌 위험 수준 (플레이어가 위기에 처한 정도)
	// 죽을 뻔한 횟수, 실패한 미션 수 등으로 증가
	// 높을수록 상품이 비싸짐 (공포 마진)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
	float GlobalRiskLevel = 0.0f;

	// 모든 마켓 아이템 데이터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	TArray<FMarketItem> MarketItems;

	// 최대 인플레이션 (난이도 상한)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	float MaxInflationIndex = 2.0f;

	// 최소 가격 배수 (가격이 이 이상으로 내려가지 않음)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	float MinPriceMultiplier = 0.5f;

	// 플레이어 계좌 잔액 (소지금)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wallet")
	float CurrentAccountBalance = 1000.0f;

	// 초기 자금 (리셋용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	float InitialBalance = 1000.0f;

	// ==================== Events ====================

	// 경제 상태가 변했을 때
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEconomyChanged, float, NewInflation, float, NewRiskLevel);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEconomyChanged OnEconomyChanged;

	// 아이템 가격이 변했을 때
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPriceChanged, FName, ItemID, float, NewPrice, float, Scarcity);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPriceChanged OnPriceChanged;

	// 계좌 잔액이 변했을 때
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBalanceChanged, float, NewBalance, bool, bIsDeposit);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBalanceChanged OnBalanceChanged;

	// 자금이 부족할 때 (구매 실패)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInsufficientFunds, float, RequiredAmount, float, CurrentBalance);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInsufficientFunds OnInsufficientFunds;

	// ==================== Methods ====================

	/**
	 * 특정 아이템의 동적 가격 계산
	 * @param ItemID: 아이템 ID
	 * @return 최종 가격
	 * 
	 * 공식:
	 * FinalPrice = BasePrice * Scarcity * CurrentInflationIndex * (1 + GlobalRiskLevel * 0.1)
	 */
	UFUNCTION(BlueprintPure, Category = "Economy")
	float GetDynamicPrice(const FName& ItemID);

	/**
	 * 인플레이션 지수 증가 (스테이지 클리어 시 호출)
	 * @param IncreaseAmount: 증가량 (기본 0.1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AdvanceInflation(float IncreaseAmount = 0.1f);

	/**
	 * 글로벌 위험 수준 증가 (위기 상황 발생 시 호출)
	 * @param RiskAmount: 증가량
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void IncreaseGlobalRisk(float RiskAmount = 0.1f);

	/**
	 * 글로벌 위험 수준 감소 (성공 후 회복)
	 * @param RecoveryAmount: 감소량
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void DecreaseGlobalRisk(float RecoveryAmount = 0.05f);

	/**
	 * 특정 아이템의 재고 수정
	 * @param ItemID: 아이템 ID
	 * @param QuantityChange: 변경 수량 (음수면 감소)
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void ModifyStock(const FName& ItemID, int32 QuantityChange);

	/**
	 * 특정 아이템의 현재 재고 조회
	 */
	UFUNCTION(BlueprintPure, Category = "Economy")
	int32 GetStock(const FName& ItemID) const;

	/**
	 * 특정 아이템의 희소성 조회
	 */
	UFUNCTION(BlueprintPure, Category = "Economy")
	float GetScarcity(const FName& ItemID) const;

	/**
	 * 경제 시스템 리셋 (새 게임 시작)
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void ResetEconomy();

	// ==================== Wallet Methods ====================

	/**
	 * 계좌에 자금 입금
	 * @param Amount: 입금 금액
	 */
	UFUNCTION(BlueprintCallable, Category = "Wallet")
	void DepositFunds(float Amount);

	/**
	 * 계좌에서 자금 인출 시도
	 * @param Amount: 인출할 금액
	 * @return 인출 성공 여부 (충분한 잔액이 있으면 true, 자금 차감)
	 */
	UFUNCTION(BlueprintCallable, Category = "Wallet")
	bool TrySpendFunds(float Amount);

	/**
	 * 현재 계좌 잔액 조회
	 */
	UFUNCTION(BlueprintPure, Category = "Wallet")
	float GetCurrentBalance() const { return CurrentAccountBalance; }

	/**
	 * 계좌 잔액을 직접 설정 (디버그/초기화용)
	 */
	UFUNCTION(BlueprintCallable, Category = "Wallet")
	void SetBalance(float NewBalance);

private:
	/**
	 * 마켓 아이템 찾기
	 */
	FMarketItem* FindMarketItem(const FName& ItemID);

	const FMarketItem* FindMarketItem(const FName& ItemID) const;

	/**
	 * 모든 아이템의 가격 업데이트 (이벤트 발생)
	 */
	void UpdateAllPrices();
};
