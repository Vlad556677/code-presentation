// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Main_Interface.h"
#include "Item_Inspectable.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USpotLightComponent;

USTRUCT(BlueprintType)
struct FInspectSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	bool bEnableInspectLight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	float IntensityLight = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	float MinPitch = -80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	float MaxPitch = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	bool bLimitYaw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings", meta = (EditCondition = "bLimitYaw"))
	float MinYaw = -90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings", meta = (EditCondition = "bLimitYaw"))
	float MaxYaw = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	float ResetRotationDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	bool bAutoRotateMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	float AutoRotateSpeed = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	float ClouseUpArmLength = 25.0f;
};

UCLASS()
class BETA_LIGHTHOUSE_API AItem_Inspectable : public AActor, public IMain_Interface
{
	GENERATED_BODY()

public:

	AItem_Inspectable();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* ItemCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USpotLightComponent* InspectLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	bool bEnableOutline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Config")
	FInspectSettings Config;

	virtual void Interact(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void OnFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void OnLostFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void Inspect(AActor* Interactor, UPrimitiveComponent* HitComponent) override;

	void RotateCamera(float PitchInput, float YawInput);
	void StopInspect();

private:

	struct FOriginalStateCache
	{
		float ArmLength;
		float Fstop;
		FRotator ActorRotation;
		FRotator SpringArmRelativeRotation;
	} DefaultState;

	AActor* CurrentInteractor = nullptr;

	FTimerHandle ResetRotationTimerHandle;
	FRotator CurrentInspectRotation;

	void ResetCameraRotation();

protected:
	virtual void BeginPlay() override;
};
