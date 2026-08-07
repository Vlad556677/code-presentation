// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Main_Interface.h"
#include "EvidenceItem.generated.h"

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
	bool bAutoRotateMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	bool bLimitYaw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	float IntensityLight = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	float MinPitch = -80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	float MaxPitch = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings", meta = (EditCondition = "bLimitYaw"))
	float MinYaw = -90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings", meta = (EditCondition = "bLimitYaw"))
	float MaxYaw = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	float ResetRotationDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	float AutoRotateSpeed = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	float ClouseUpArmLength = 25.0f;
};

UCLASS()
class BETA_LIGHTHOUSE_API AEvidenceItem : public AActor, public IMain_Interface
{
	GENERATED_BODY()

///=======================
///       Ã≈“Œƒ€
///=======================

public:
	AEvidenceItem();

	void RotateCamera(float PitchInput, float YawInput);
	void StopInspect();

private:
	void ResetCameraRotation();

///=======================
///   VIRTUALññÃ≈“Œƒ€
///=======================

public:
	virtual void Interact(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void OnFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void OnLostFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) override;
	virtual void Inspect(AActor* Interactor, UPrimitiveComponent* HitComponent) override;

protected:
	virtual void BeginPlay() override;

///=======================
///       œŒÀﬂ
///=======================

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* ItemCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USpotLightComponent* InspectLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Settings")
	bool bEnableOutline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect Config")
	FInspectSettings Config;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Inspect Settings")
	UMaterialInterface* OverlayMaterial;

private:
	struct FOriginalStateCache
	{
		float ArmLength;
		float Fstop;
		FRotator ActorRotation;
		FRotator SpringArmRelativeRotation;
	} DefaultState;

	AActor* CurrentInteractor;
	FTimerHandle ResetRotationTimerHandle;
	FRotator CurrentInspectRotation;
};
