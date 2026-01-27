// Copyright 2024, Korstian Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentComponent.generated.h"

class UWeaponDataAsset;
class ABaseWeapon;
class ACharacter;

/**
 * @class UEquipmentComponent
 * @brief 캐릭터의 장비(특히 무기)를 관리하는 컴포넌트입니다.
 * 무기 교체, 현재 장착된 무기 액터 및 관련 데이터 애셋에 대한 접근을 담당합니다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEFRAMEWORKTEST_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEquipmentComponent();

protected:
	virtual void BeginPlay() override;

private:
	/** 
	 * @brief 현재 장착된 무기의 데이터 애셋(UWeaponDataAsset)에 대한 포인터입니다.
	 * 공격 몽타주, 대미지 배율, 스태미나 소모량 등 모든 비주얼/로직 데이터를 담고 있습니다.
	 * CombatComponent에서 이 데이터를 참조하여 실제 공격 로직을 수행합니다.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Equipment")
	TObjectPtr<UWeaponDataAsset> CurrentWeaponData;

	/**
	 * @brief 현재 월드에 스폰되어 캐릭터에게 부착된 무기 액터에 대한 포인터입니다.
	 * 무기의 시각적 표현을 담당하며, 공격 판정(Trace)의 시작점이 될 수 있습니다.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Equipment")
	TObjectPtr<AActor> CurrentWeaponActor;
	
	/** @brief 이 컴포넌트를 소유한 캐릭터에 대한 포인터입니다. */
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

public:
	/**
	 * @brief 제공된 데이터 애셋을 기반으로 새로운 무기를 장착합니다.
	 * 이전에 장착된 무기 액터가 있다면 파괴하고, 새로운 무기 액터를 스폰하여 캐릭터의 지정된 소켓에 부착합니다.
	 * @param WeaponToEquip 장착할 무기의 데이터가 담긴 UWeaponDataAsset.
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipWeapon(UWeaponDataAsset* WeaponToEquip);

	/**
	 * @brief 현재 장착된 무기의 데이터 애셋을 가져옵니다.
	 * @return 현재 무기 데이터 애셋. 무기가 없다면 nullptr를 반환합니다.
	 */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	UWeaponDataAsset* GetCurrentWeaponData() const { return CurrentWeaponData; }

	/**
	 * @brief 현재 장착된 무기의 액터를 가져옵니다.
	 * @return 현재 무기 액터. 무기가 없다면 nullptr를 반환합니다.
	 */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	AActor* GetCurrentWeaponActor() const { return CurrentWeaponActor; }
};
