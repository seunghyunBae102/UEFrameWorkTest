// Copyright 2024, Korstian Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

class USceneComponent;
class UStaticMeshComponent;

/**
 * @class ABaseWeapon
 * @brief 게임에 등장하는 모든 무기 액터의 기반이 되는 클래스입니다.
 * 이 클래스는 무기의 시각적 표현(메쉬)과 공격 판정에 사용될 트레이스의 시작/끝 위치를 정의합니다.
 * EquipmentComponent에 의해 스폰되고 캐릭터에 부착됩니다.
 */
UCLASS()
class UEFRAMEWORKTEST_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseWeapon();

protected:
	/** @brief 액터의 루트 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> Root;
	
	/** @brief 무기의 외형을 나타내는 스태틱 메쉬 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	/**

	 * @brief 공격 판정을 위한 트레이스의 시작 지점으로 사용될 소켓(씬 컴포넌트)입니다.
	 * 보통 무기 자루의 윗부분에 위치시킵니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Trace")
	TObjectPtr<USceneComponent> TraceStart;

	/**
	 * @brief 공격 판정을 위한 트레이스의 끝 지점으로 사용될 소켓(씬 컴포넌트)입니다.
	 * 보통 무기 칼날의 끝부분에 위치시킵니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Trace")
	TObjectPtr<USceneComponent> TraceEnd;

public:
	/** @brief TraceStart 소켓의 월드 위치를 반환합니다. CombatComponent의 PerformWeaponTrace에서 사용됩니다. */
	FORCEINLINE FVector GetTraceStartLocation() const { return TraceStart->GetComponentLocation(); }
	
	/** @brief TraceEnd 소켓의 월드 위치를 반환합니다. CombatComponent의 PerformWeaponTrace에서 사용됩니다. */
	FORCEINLINE FVector GetTraceEndLocation() const { return TraceEnd->GetComponentLocation(); }

};
