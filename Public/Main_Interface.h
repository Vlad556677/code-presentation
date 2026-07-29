// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Main_Interface.generated.h"

class AActor;
class UPrimitiveComponent;

UINTERFACE(MinimalAPI)
class UMain_Interface : public UInterface
{
	GENERATED_BODY()
};


class BETA_LIGHTHOUSE_API IMain_Interface
{
	GENERATED_BODY()

public:
	virtual void Interact(AActor* Interactor, UPrimitiveComponent* HitComponent) = 0;
	virtual void OnFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) = 0;
	virtual void OnLostFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) = 0;
	virtual void Inspect(AActor* Interactor, UPrimitiveComponent* HitComponent) = 0;
};
