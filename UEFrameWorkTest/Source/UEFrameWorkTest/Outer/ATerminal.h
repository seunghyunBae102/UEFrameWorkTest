// GHOST IN THE MONITOR - Terminal Actor (Outer World)
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/BPI_Interactable.h"
#include "ATerminal.generated.h"

class UDA_ShopItem;

/**
 * ATerminal - Outer World의 PC 터미널 (상점)
 * 
 * 역할:
 * - 플레이어가 탄약/부품을 구매하는 인터페이스
 * - BPI_Interactable를 구현하여 상호작용 가능
 * - 상점 UI 열기 및 구매 로직 위임
 * 
 * 규칙:
 * - 최소한의 구조만 유지 (Event Broadcaster 역할)
 * - 구매 로직은 Subsystem_EconomyManager의 TrySpendFunds() 호출
 * - UI 표시는 BlueprintImplementableEvent로 위임
 */
UCLASS()
class UEFRAMEWORKTEST_API ATerminal : public AActor, public IBP_Interactable
{
	GENERATED_BODY()

public:
	ATerminal();

	virtual void BeginPlay() override;

	// ==================== Properties ====================

	// 터미널의 시각적 표현 (메시)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	class UStaticMeshComponent* MeshComponent;

	// 충돌 감지용 (상호작용 범위)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	class USphereComponent* InteractionSphere;

	// 이 터미널이 판매하는 모든 아이템
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TArray<UDA_ShopItem*> AvailableItems;

	// 터미널 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText TerminalName = FText::FromString(TEXT("Supply Terminal"));

	// 상점이 열려 있는지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop")
	bool bShopOpen = false;

	// ==================== Events ====================

	// 상점이 열렸을 때
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopOpened);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnShopOpened OnShopOpened;

	// 상점이 닫혔을 때
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopClosed);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnShopClosed OnShopClosed;

	// 구매 완료 시 발생
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPurchaseComplete, UDA_ShopItem*, PurchasedItem, float, RemainingBalance);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPurchaseComplete OnPurchaseComplete;

	// ==================== Interface Implementation ====================

	virtual void OnInteract_Implementation(AActor* Interactor) override;

	virtual bool CanInteract_Implementation() const override;

	virtual FText GetInteractionPrompt_Implementation() const override;

	// ==================== Methods ====================

	/**
	 * 특정 상품 구매 시도
	 * @param Item: 구매할 상품
	 * @param Buyer: 구매자 (플레이어)
	 * @return 구매 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	bool PurchaseItem(UDA_ShopItem* Item, AActor* Buyer);

	/**
	 * 상점 열기 (UI 표시)
	 * Blueprint에서 구현
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Shop")
	void OpenShopUI();

	/**
	 * 상점 닫기 (UI 숨김)
	 * Blueprint에서 구현
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Shop")
	void CloseShopUI();

	/**
	 * 상점의 판매 가능 상품 목록 반환
	 */
	UFUNCTION(BlueprintPure, Category = "Shop")
	TArray<UDA_ShopItem*> GetAvailableItems() const { return AvailableItems; }

private:
	/**
	 * 배송 시뮬레이션 타이머 콜백
	 */
	void OnDeliveryComplete();
};
