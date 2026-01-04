// GHOST IN THE MONITOR - Interactable Interface
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BPI_Interactable.generated.h"

/**
 * BPI_Interactable - 상호작용 인터페이스
 * 
 * 역할:
 * - Inner/Outer World 모두에서 사용 가능한 일반적인 상호작용 정의
 * - AGoldExchange, ATerminal 등이 구현
 * - 플레이어가 상호작용 가능한 객체를 정의
 * 
 * 규칙:
 * - Cast 없이 Interface Call로만 통신
 * - 상호작용 로직은 Component에 위임
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UBP_Interactable : public UInterface
{
	GENERATED_BODY()
};

class IBP_Interactable
{
	GENERATED_BODY()

public:
	/**
	 * 객체와 상호작용
	 * @param Interactor: 상호작용을 시도하는 액터 (플레이어)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteract(AActor* Interactor);

	/**
	 * 상호작용 가능 여부 확인
	 * @return 상호작용 가능하면 true
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract() const;

	/**
	 * 상호작용 프롬프트 텍스트 반환
	 * (UI에 "Press E to use Terminal" 같은 메시지 표시)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionPrompt() const;
};
