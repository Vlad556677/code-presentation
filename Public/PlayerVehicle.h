// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Main_Interface.h"
#include "PlayerVehicle.generated.h"

class AFuelPump;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UChaosWheeledVehicleMovementComponent;
class UStaticMesh;
class ADetectiveCharacter;
class UBoxComponent;
class USceneComponent;
struct FInputActionValue;

// Делегат для уведомления других систем (например, UI спидометра), что ручник был нажат/отпущен
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHandbrakeChangedSignature, bool, bIsActive);

USTRUCT(BlueprintType)
struct FFirstPersonCarSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float BrakeToReverseThreshold = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float MaxCameraYaw = 140.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float MaxCameraPitch = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float CoastingBrakeFunction = 0.1f; //легкое торможение при отпускании педали
};

UCLASS()
class BETA_LIGHTHOUSE_API APlayerVehicle : public AWheeledVehiclePawn, public IMain_Interface
{
	GENERATED_BODY()
		
///=======================
///       МЕТОДЫ
///=======================

public:
	APlayerVehicle();

	void EnterVehicle(ADetectiveCharacter* PlayerCharacter);
	void ExitVehicle(const FInputActionValue& Value);
	void SetRefuelingState(bool bState){ bIsRefueling = bState; }
	void ToggleGasCover();
	UBoxComponent* GetGasTankTrigger() const { return GasTankTrigger; }

protected:
	void UpdateSpeedometr();
	void UpdateSteeringWheel();

private:
	void ApplyThrottle(const FInputActionValue& Value);
	void ApplySteering(const FInputActionValue& Value);
	void ApplyInteractive(const FInputActionValue& Value);
	void ApplyLook(const FInputActionValue& Value);
	void HandbrakeHandler(const FInputActionValue& Value);

///=======================
///   VIRTUAL––МЕТОДЫ
///=======================

public:
	virtual void Interact(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void OnFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void OnLostFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void Inspect(AActor* Interactor, UPrimitiveComponent* HitComponent) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputPlayerComponent) override;

///=======================
///       ПОЛЯ
///=======================

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHandbrakeChangedSignature OnHandbrakeChanged;

	//===-Box-Component-===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* GasTankTrigger;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* DoorTrigger;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* GasCoverMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* GasHoleLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	AFuelPump* ConnectedPump = nullptr;

	USceneComponent* GetGasHoleLocation() const{ return GasHoleLocation; }

	bool IsGasCoverOpen() const { return bIsGasCoverOpen; }
	bool GetRefuelingState() const { return bIsRefueling; }
	
protected:
	//===-Componenta-===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CarCamera;

	//===-Settings-===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FFirstPersonCarSettings CarSettings;

	//===-Input-Action-===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input_Action")
	UInputAction* ThrottleAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input_Action")
	UInputAction* SteeringAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input_Action")
	UInputAction* HandbrakeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input_Action")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input_Action")
	UInputAction* OpenDoorAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input_Mapping")
	UInputMappingContext* InputMapping;

	//===-Skeletal-Mesh-===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Static_Mesh")
	UStaticMeshComponent* RulMesh;
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Static_Mesh")
	UStaticMeshComponent* LeftDoorMesh;
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Static_Mesh")
	UStaticMeshComponent* WindowSpeedMesh;
	
	//===-Timer-Handle-===
	FTimerHandle SpeedometrTimerHandle;
	FTimerHandle SteeringTimerHandle;

	//===-Settings-Timer-===
	UPROPERTY(EditAnywhere,Category="Car UI")
	float MaxSpeedForGauge;
	UPROPERTY(EditAnywhere,Category="Car UI")
	float MaxGaugeAngle;
	UPROPERTY(EditAnywhere,Category="Car UI")
	float MaxRulRotation;

	UPROPERTY()
	ADetectiveCharacter* StoredPlayer;

private:
	bool IsMovingForward() const;

	UPROPERTY()
	UChaosWheeledVehicleMovementComponent* VehicleMovement;

	bool bIsGasCoverOpen;
	bool bIsRefueling;
};
