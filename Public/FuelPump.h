// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Main_Interface.h"
#include "FuelPump.generated.h"

class UBoxComponent;
class UCableComponent;
class APlayerVehicle;

UCLASS()
class BETA_LIGHTHOUSE_API AFuelPump : public AActor, public IMain_Interface
{
	GENERATED_BODY()
	
///=======================
///       Ã≈“Œƒ€
///=======================

public:	
	AFuelPump();

	void TogglePistolet();

///=======================
///   VIRTUALññÃ≈“Œƒ€
///=======================

public:
	virtual void Interact(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void OnFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void OnLostFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void Inspect(AActor* Interactor, UPrimitiveComponent* HitComponent) override;

	virtual void BeginPlay() override;

///=======================
///       œŒÀﬂ
///=======================

public:
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

	class APlayerVehicle* CurrentRefuelingCar;

	UPROPERTY(EditAnywhere,Category = "Components")
	float MaxCableLeght;
	
private:
	bool bIsAttecheToCar;

};
