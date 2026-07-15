// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Main_Interface.h"
#include "VWP_PlayerCar.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UChaosWheeledVehicleMovementComponent;
class UStaticMesh;
class AMain_Player;
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
class BETA_LIGHTHOUSE_API AVWP_PlayerCar : public AWheeledVehiclePawn, public IMain_Interface
{
	GENERATED_BODY()
	
public:
	AVWP_PlayerCar();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHandbrakeChangedSignature OnHandbrakeChanged;

	void EnterVehicle(AMain_Player* PlayerCharacter);
	void ExitVehicle(const FInputActionValue& Value);

	//===-Box-Component-===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	UBoxComponent* GasTankTrigger;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Static_Mesh")
	UStaticMeshComponent* GasCoverMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* GasHoleLocation;


	virtual void Interact(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void OnFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void OnLostFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void Inspect(AActor* Interactor, UPrimitiveComponent* HitComponent) override;


	bool IsGasCoverOpen() const { return bIsGasCoverOpen; }
	USceneComponent* GetGasHoleLocation() const{ return GasHoleLocation; }

	void SetRefuelingState(bool bState){ bIsRefueling = bState; }
	bool GetRefuelingState() const { return bIsRefueling; }
	


	//===-Function-===
	void ToggleGasCover();

	

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputPlayerComponent) override;

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
	void UpdateSpeedometr();
	FTimerHandle SteeringTimerHandle;
	void UpdateSteeringWheel();
		//===-Settings-Timer-===
	UPROPERTY(EditAnywhere,Category="Car UI")
	float MaxSpeedForGauge = 200.0f;
	UPROPERTY(EditAnywhere,Category="Car UI")
	float MaxGaugeAngle = 270.0f;
	UPROPERTY(EditAnywhere,Category="Car UI")
	float MaxRulRotation = 65.0f;

	UPROPERTY()
	AMain_Player* StoredPlayer;



private:



	//===-Function-Input-===
	void ApplyThrottle(const FInputActionValue& Value);
	void ApplySteering(const FInputActionValue& Value);
	void ApplyInteractive(const FInputActionValue& Value);
	void ApplyLook(const FInputActionValue& Value);
	void HandbrakeHandler(const FInputActionValue& Value);

	bool IsMovingForward() const;

	UPROPERTY()
	UChaosWheeledVehicleMovementComponent* VehicleMovement;

	//===-Variables-===
	bool bIsGasCoverOpen = false;
	bool bIsRefueling = false;
};
