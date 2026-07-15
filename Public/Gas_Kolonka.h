// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Main_Interface.h"
#include "Gas_Kolonka.generated.h"

class UBoxComponent;
class UCableComponent;
class AVWP_PlayerCar;

UCLASS()
class BETA_LIGHTHOUSE_API AGas_Kolonka : public AActor, public IMain_Interface
{
	GENERATED_BODY()
	
public:	

	AGas_Kolonka();

	virtual void Interact(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void OnFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void OnLostFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void Inspect(AActor* Interactor, UPrimitiveComponent* HitComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* BaseMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* Pistolet;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UCableComponent* CableMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UBoxComponent* InteractTrigger;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* IdleNozzleLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshHole;

	class AVWP_PlayerCar* CurrentRefuelingCar = nullptr;

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere,Category = "Components")
	float MaxCableLeght = 500.0f;

	void TogglePistolet();
private:
	bool bIsAttecheToCar = false;
};
