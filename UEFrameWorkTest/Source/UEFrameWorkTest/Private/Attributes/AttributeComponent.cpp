// Copyright 2024, Korstian Project. All Rights Reserved.

#include "Attributes/AttributeComponent.h"
#include "Engine/DataTable.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsDirty = true;
	BaseStatTable = nullptr;
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeBaseStats();
}

void UAttributeComponent::InitializeBaseStats()
{
	if (!BaseStatTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttributeComponent: BaseStatTable is not set. Cannot initialize attributes."));
		return;
	}

	TArray<FAttributeData*> AllRows;
	BaseStatTable->GetAllRows(TEXT("Initializing Base Stats"), AllRows);

	for (const FAttributeData* Row : AllRows)
	{
		if (Row)
		{
			AttributeMap.Add(Row->AttributeType, FModifiableAttribute(Row->BaseValue));
		}
	}
	
	bIsDirty = true;
	// Force a recalculation and broadcast initial values
	for (auto& Elem : AttributeMap)
	{
		GetAttributeValue(Elem.Key);
	}
}

void UAttributeComponent::AddModifier(EAttributeType Type, const FStatModifier& Mod)
{
	if (FModifiableAttribute* Attr = AttributeMap.Find(Type))
	{
		Attr->Modifiers.Add(Mod);
		bIsDirty = true;

		// Immediately recalculate and broadcast the change
		GetAttributeValue(Type);
	}
}

void UAttributeComponent::RemoveModifiersBySource(UObject* Source)
{
	if (!Source) return;

	TArray<EAttributeType> AffectedAttributes;

	for (auto& Elem : AttributeMap)
	{
		int32 RemovedCount = Elem.Value.Modifiers.RemoveAll([&](const FStatModifier& Mod)
		{
			return Mod.Source.Get() == Source;
		});

		if (RemovedCount > 0)
		{
			AffectedAttributes.Add(Elem.Key);
		}
	}
	
	if (AffectedAttributes.Num() > 0)
	{
		bIsDirty = true;
		for (EAttributeType AffectedType : AffectedAttributes)
		{
			// Recalculate and broadcast changes for all affected attributes
			GetAttributeValue(AffectedType);
		}
	}
}


float UAttributeComponent::GetAttributeValue(EAttributeType Type)
{
	FModifiableAttribute* Attr = AttributeMap.Find(Type);
	if (!Attr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttributeComponent: Tried to get value for an unknown attribute type %s"), *UEnum::GetValueAsString(Type));
		return 0.0f;
	}

	if (bIsDirty)
	{
		// In a more complex scenario, we'd only recalculate the specific dirty attribute.
		// For simplicity and based on the spec, recalculating all is acceptable for now.
		// A better approach is to make attributes individually dirty.
		for (auto& Elem : AttributeMap)
		{
			float OldValue = Elem.Value.GetCurrentValue();
			RecalculateAttribute(Elem.Value);
			BroadcastAttributeChange(Elem.Key, OldValue, Elem.Value.GetCurrentValue());
		}
		bIsDirty = false;
	}

	return Attr->GetCurrentValue();
}


float UAttributeComponent::GetBaseAttributeValue(EAttributeType Type)
{
	if (const FModifiableAttribute* Attr = AttributeMap.Find(Type))
	{
		return Attr->GetBaseValue();
	}

	UE_LOG(LogTemp, Warning, TEXT("AttributeComponent: Tried to get base value for an unknown attribute type %s"), *UEnum::GetValueAsString(Type));
	return 0.0f;
}


void UAttributeComponent::RecalculateAttribute(FModifiableAttribute& AttributeToUpdate)
{
	float AdditiveTotal = 0.f;
	float MultiplicativeTotal = 1.f;

	for (const FStatModifier& Mod : AttributeToUpdate.Modifiers)
	{
		if (Mod.Type == EModifierType::Add)
		{
			AdditiveTotal += Mod.Value;
		}
		else if (Mod.Type == EModifierType::Multiply)
		{
			MultiplicativeTotal *= Mod.Value;
		}
	}

	// Formula: (Base + Additive) * Multiplicative
	float NewValue = (AttributeToUpdate.GetBaseValue() + AdditiveTotal) * MultiplicativeTotal;

	// Clamping to prevent negative values for relevant stats
	if (NewValue < 0.f)
	{
		// A more robust system might have per-attribute rules for clamping.
		NewValue = 0.f;
	}
	
	AttributeToUpdate.SetCurrentValue(NewValue);
}

void UAttributeComponent::BroadcastAttributeChange(EAttributeType Type, float OldValue, float NewValue)
{
	if (FMath::IsNearlyEqual(OldValue, NewValue))
	{
		return;
	}

	float Delta = NewValue - OldValue;
	OnAttributeChanged.Broadcast(Type, NewValue, Delta);

	// If poise just dropped to or below zero, broadcast depletion
	if (Type == EAttributeType::Poise && OldValue > 0.f && NewValue <= 0.f)
	{
		OnPoiseDepleted.Broadcast();
	}
}
