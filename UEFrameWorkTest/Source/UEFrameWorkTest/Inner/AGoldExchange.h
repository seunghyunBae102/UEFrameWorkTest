// GHOST IN THE MONITOR - Gold Exchange Actor (Inner World)
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/BPI_Interactable.h"
#include "AGoldExchange.generated.h"

/**
 * AGoldExchange - Inner World의 금화 거래소
 * 
 * 역할:
 * - 플레이어가 소지한 금화를 은행에 입금하는 NPC/객체
 * - BPI_Interactable를 구현하여 상호작용 가능
 * - 실제 로직은 Subsystem_EconomyManager에 위임
 * 
 * 규칙:
 * - 최소한의 구조만 유지 (Event Broadcaster 역할)
 * - 복잡한 로직은 컴포넌트나 서브시스템에 위임
 * - BeginPlay에서 초기화, OnInteract에서 이벤트 발생
 */
UCLASS()
class UEFRAMEWORKTEST_API AGoldExchange : public AActor, public IBP_Interactable
{
	GENERATED_BODY()

public:
	AGoldExchange();

	virtual void BeginPlay() override;

	// ==================== Properties ====================

	// 거래소의 시각적 표현 (메시)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	class UStaticMeshComponent* MeshComponent;

	// 충돌 감지용 (상호작용 범위)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	class USphereComponent* InteractionSphere;

	// 한 번에 입금할 금액 (기본값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exchange")
	float DepositAmount = 100.0f;

	// 거래소 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText ExchangeName = FText::FromString(TEXT("Gold Exchange"));

	// ==================== Events ====================

	// 입금 완료 시 발생
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDepositComplete, float, DepositedAmount, float, NewBalance);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDepositComplete OnDepositComplete;

	// ==================== Interface Implementation ====================

	virtual void OnInteract_Implementation(AActor* Interactor) override;

	virtual bool CanInteract_Implementation() const override;

	virtual FText GetInteractionPrompt_Implementation() const override;

private:
	/**
	 * 플레이어 금화 입금 처리
	 */
	void ProcessDeposit(AActor* Player);
};
