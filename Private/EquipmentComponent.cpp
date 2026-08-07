// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentComponent.h"
#include "EquippableItem.h"

UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEquipmentComponent::EquipAnItem(const TSubclassOf<AEquippableItem> ItemClassToEquip, USceneComponent* const LabelToAttach)
{
	// hiding the current item, if one exists
	if (CurrentSubject)
	{
		CurrentSubject->Unequip();
		CurrentSubject->SetActorHiddenInGame(true);
	}

	// searching for an item in the "pocket" to avoid spawning it again
	AEquippableItem* const* const Indicator = Pocket.Find(ItemClassToEquip);

	if (Indicator)
	{
		// pulled an object out of the "pocket"
		CurrentSubject = *Indicator;
	}
	else
	{
		// Spawned item for the first time + added to inventory.
		AEquippableItem* const NewItem = GetWorld()->SpawnActor<AEquippableItem>(ItemClassToEquip);
		NewItem->SetOwner(GetOwner());

		Pocket.Add(ItemClassToEquip, NewItem);
		CurrentSubject = NewItem;
	}
	// display of the object
	CurrentSubject->SetActorHiddenInGame(false);
	CurrentSubject->Equip(LabelToAttach);
}

void UEquipmentComponent::UseCurrentItem()
{
	if (CurrentSubject)
	{
		CurrentSubject->MainAction();
	}
}

void UEquipmentComponent::UnequipCurrentItem()
{
	if (CurrentSubject)
	{
		CurrentSubject->Unequip();
	}
}

void UEquipmentComponent::AdditionsActionItem(const float Number)
{
	if (CurrentSubject)
	{
		CurrentSubject->SecondaryAction(Number);
	}
}


