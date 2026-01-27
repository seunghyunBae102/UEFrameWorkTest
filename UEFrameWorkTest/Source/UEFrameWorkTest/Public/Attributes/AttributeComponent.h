// Copyright 2024, Korstian Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Attributes/AttributeSystemTypes.h"
#include "AttributeComponent.generated.h"

/**
 * @brief 스탯(Attribute) 값이 변경될 때 브로드캐스트되는 델리게이트입니다.
 * @param Type 변경된 스탯의 종류
 * @param NewValue 변경 후의 새로운 최종 값
 * @param Delta 값의 변화량 (NewValue - OldValue)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChanged, EAttributeType, Type, float, NewValue, float, Delta);

/**
 * @brief 강인도(Poise)가 0 이하로 떨어졌을 때 브로드캐스트되는 델리게이트입니다.
 * 캐릭터가 스턴 상태에 빠져야 함을 알리는 역할을 합니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPoiseDepleted);

/**
 * @class UAttributeComponent
 * @brief 캐릭터의 모든 스탯(Attribute)을 관리하는 핵심 컴포넌트입니다.
 * 스탯의 초기화, 수정, 계산 및 변경 사항 전파를 담당합니다. '스탯 시스템의 두뇌' 역할을 합니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UEFRAMEWORKTEST_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();

	/** @brief 스탯의 최종 값(CurrentValue)이 변경될 때마다 호출되는 델리게이트입니다. UI(체력 바), 무브먼트(이동 속도) 등 다른 시스템이 스탯 변화에 반응하는 데 사용됩니다. */
	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnAttributeChanged OnAttributeChanged;

	/** @brief 강인도가 0 이하로 떨어졌을 때 호출되는 델리게이트입니다. 캐릭터가 스턴 상태에 들어가도록 하는 데 사용됩니다. */
	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnPoiseDepleted OnPoiseDepleted;

protected:
	virtual void BeginPlay() override;

private:
	/** @brief 이 컴포넌트가 관리하는 모든 스탯 데이터를 저장하는 맵입니다. EAttributeType을 키로 사용하여 각 스탯의 FModifiableAttribute 데이터에 접근합니다. */
	UPROPERTY(VisibleAnywhere, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	TMap<EAttributeType, FModifiableAttribute> AttributeMap;

	/** @brief 스탯 값의 재계산이 필요한지 여부를 나타내는 플래그입니다. 모디파이어가 추가되거나 제거될 때 true로 설정됩니다. */
	UPROPERTY(VisibleAnywhere, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	bool bIsDirty;

	/** @brief 컴포넌트 초기화 시 기본 스탯 값을 설정하는 데 사용될 데이터 테이블입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> BaseStatTable;
	
public:
	/**
	 * @brief BaseStatTable에 지정된 데이터 테이블을 읽어 모든 스탯의 기본 값을 초기화합니다.
	 * 보통 BeginPlay에서 자동으로 호출됩니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void InitializeBaseStats();

	/**
	 * @brief 특정 스탯에 새로운 모디파이어를 추가합니다.
	 * 즉시 적용되지 않으며, 다음에 GetAttributeValue가 호출될 때 재계산이 이루어집니다.
	 * @param Type 모디파이어를 적용할 스탯의 종류
	 * @param Mod 적용할 모디파이어 데이터
	 */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void AddModifier(EAttributeType Type, const FStatModifier& Mod);

	/**
	 * @brief 특정 출처(Source)로부터 비롯된 모든 모디파이어를 제거합니다.
	 * 예를 들어, 아이템 장착을 해제하거나 버프 효과가 만료될 때 사용됩니다.
	 * @param Source 제거할 모디파이어들의 출처가 되는 오브젝트
	 */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void RemoveModifiersBySource(UObject* Source);

	/**
	 * @brief 특정 스탯의 최종 계산된 값을 가져옵니다.
	 * 만약 bIsDirty 플래그가 true라면, 값을 반환하기 전에 재계산을 먼저 수행합니다.
	 * @param Type 값을 가져올 스탯의 종류
	 * @return 해당 스탯의 최종 값 (BaseValue + 모든 모디파이어 적용 후)
	 */
	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetAttributeValue(EAttributeType Type);

	/**
	 * @brief 특정 스탯의 모디파이어가 적용되지 않은 순수 기본 값을 가져옵니다.
	 * @param Type 기본 값을 가져올 스탯의 종류
	 * @return 해당 스탯의 BaseValue
	 */
	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetBaseAttributeValue(EAttributeType Type);

private:
	/**
	 * @brief 특정 스탯의 최종 값(CurrentValue)을 재계산합니다.
	 * BaseValue에 모든 합연산(Add) 모디파이어를 더하고, 그 결과에 모든 곱연산(Multiply) 모디파이어를 곱합니다.
	 * @param AttributeToUpdate 재계산할 스탯 데이터에 대한 참조
	 */
	void RecalculateAttribute(FModifiableAttribute& AttributeToUpdate);

	/**
	 * @brief 스탯 값의 변경을 OnAttributeChanged 델리게이트를 통해 외부에 알립니다(브로드캐스트).
	 * @param Type 변경된 스탯의 종류
	 * @param OldValue 변경 전의 값
	 * @param NewValue 변경 후의 새로운 값
	 */
	void BroadcastAttributeChange(EAttributeType Type, float OldValue, float NewValue);
};
