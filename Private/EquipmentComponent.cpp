// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentComponent.h"
#include "EquippableItem.h"

UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEquipmentComponent::EquipAnItem(const TSubclassOf<AEquippableItem> ItemClassToEquip, USceneComponent* const LabelToAttach)
{
	// скрытие текущего предмета, если он есть
	if (CurrentSubject)
	{
		CurrentSubject->Unequip();
		CurrentSubject->SetActorHiddenInGame(true);
	}

	// поиск предмета в "кармане", чтобы не спавнить его заново
	AEquippableItem* const* const Indicator = Pocket.Find(ItemClassToEquip);

	if (Indicator)
	{
		// достал предмет из "кармана"
		CurrentSubject = *Indicator;
	}

	else
	{
		// спавн предмет впервые + добавил в карман
		AEquippableItem* const NewItem = GetWorld()->SpawnActor<AEquippableItem>(ItemClassToEquip);
		NewItem->SetOwner(GetOwner());

		Pocket.Add(ItemClassToEquip, NewItem);
		CurrentSubject = NewItem;
	}
	// показ предмета 
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


