// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentComponent.h"


UEquipmentComponent::UEquipmentComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

}


void UEquipmentComponent::EquipAnItem(TSubclassOf<AEquippableItem> ItemClassToEquip, USceneComponent* LabelToAttach)
{
	if (CurrentSubject != nullptr)
	{
		CurrentSubject->Unequip();
		CurrentSubject->SetActorHiddenInGame(true);
	}
	AEquippableItem** Indicator = Pocket.Find(ItemClassToEquip);

	if (Indicator != nullptr)
	{
		CurrentSubject = *Indicator;
	}

	else
	{
		AEquippableItem* NewItem = GetWorld()->SpawnActor<AEquippableItem>(ItemClassToEquip);
		NewItem->SetOwner(GetOwner());

		Pocket.Add(ItemClassToEquip, NewItem);
		CurrentSubject = NewItem;
	}
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
	if (CurrentSubject != nullptr)
	{
		CurrentSubject->Unequip();
	}
}

void UEquipmentComponent::AdditionsActionItem(float Number)
{
	if (CurrentSubject)
	{
		CurrentSubject->SecondaryAction(Number);
	}
}


