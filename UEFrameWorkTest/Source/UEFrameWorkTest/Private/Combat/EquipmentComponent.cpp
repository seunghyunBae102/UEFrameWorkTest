// Copyright 2024, Korstian Project. All Rights Reserved.

#include "Combat/EquipmentComponent.h"
#include "Data/WeaponDataAsset.h"
#include "GameFramework/Character.h"
#include "Combat/BaseWeapon.h"
#include "Components/StaticMeshComponent.h"

UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentWeaponData = nullptr;
	CurrentWeaponActor = nullptr;
}

void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UEquipmentComponent::EquipWeapon(UWeaponDataAsset* WeaponToEquip)
{
	if (!OwnerCharacter || !WeaponToEquip || !WeaponToEquip->WeaponMesh)
	{
		return;
	}

	CurrentWeaponData = WeaponToEquip;

	// Destroy the old weapon actor if it exists
	if (CurrentWeaponActor)
	{
		CurrentWeaponActor->Destroy();
		CurrentWeaponActor = nullptr;
	}

	// Spawn the new weapon actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerCharacter;
	SpawnParams.Instigator = OwnerCharacter;
	// We spawn a base weapon actor, which will contain the mesh component
	CurrentWeaponActor = GetWorld()->SpawnActor<ABaseWeapon>(ABaseWeapon::StaticClass(), SpawnParams);

	if (CurrentWeaponActor)
	{
		// Find the mesh component within the spawned weapon actor
		UStaticMeshComponent* WeaponMeshComp = CurrentWeaponActor->FindComponentByClass<UStaticMeshComponent>();
		if (WeaponMeshComp)
		{
			// Set the mesh from the data asset
			WeaponMeshComp->SetStaticMesh(WeaponToEquip->WeaponMesh);
		}
		
		// Attach the new weapon to the character's mesh at the correct socket
		CurrentWeaponActor->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponToEquip->SocketName);
	}
}
