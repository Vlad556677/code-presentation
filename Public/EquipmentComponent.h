// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentComponent.generated.h"

class AEquippableItem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BETA_LIGHTHOUSE_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

///=======================
///       лерндш
///=======================

public:	
	UEquipmentComponent();

	void EquipAnItem(TSubclassOf<AEquippableItem> ItemClassToEquip, USceneComponent* LabelToAttach);
	void UseCurrentItem();
	void UnequipCurrentItem();
	void AdditionsActionItem(float Number);

///=======================
///       онкъ
///=======================

private:
	UPROPERTY()
	AEquippableItem* CurrentSubject = nullptr;

	UPROPERTY()
	TMap<TSubclassOf<AEquippableItem>, AEquippableItem*> Pocket;
};
