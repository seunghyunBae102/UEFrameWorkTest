// GHOST IN THE MONITOR - Shop Item Data Asset (Outer World)
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_ShopItem.generated.h"

/**
 * DA_ShopItem - Outer World PC 상점에서 판매하는 아이템
 * 
 * 역할:
 * - 터미널에서 구매 가능한 탄약, 부품 등을 정의
 * - 배달 시간, 가격, 아이콘 등 상점 정보 포함
 * - 실제 데이터(탄약/부품)는 TSoftObjectPtr로 참조 (메모리 최적화)
 * 
 * 규칙:
 * - 데이터만 정의 (로직 없음)
 * - 기획자가 에디터에서 쉽게 생성/수정 가능
 */
UCLASS(BlueprintType, Const)
class UEFRAMEWORKTEST_API UDA_ShopItem : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 상점에서 보여줄 아이템 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText DisplayName = FText::FromString(TEXT("New Item"));

	// 기본 판매 가격 (경제 시스템에서 동적으로 조정됨)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Price")
	float BasePrice = 100.0f;

	// 이 상품이 참조하는 실제 데이터 에셋
	// 예: DA_9mmParabellum (UDA_AmmoType) 또는 DA_SteelSword (UDA_InnerEquipment)
	// TSoftObjectPtr을 사용하여 메모리 효율성 증대
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Product")
	TSoftObjectPtr<UPrimaryDataAsset> ProductData;

	// 배송 시간 (초 단위)
	// 0 = 즉시 배송, 30 = 30초 후 배송
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delivery")
	float DeliveryTime = 0.0f;

	// 상점 UI에 표시할 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	UTexture2D* Icon = nullptr;

	// 재고 상태 (비어있으면 구매 불가)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stock")
	bool bInStock = true;

	// 상품 설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText Description;

	// 카테고리 (Ammo, Parts, Equipment 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FName Category = TEXT("General");
};
