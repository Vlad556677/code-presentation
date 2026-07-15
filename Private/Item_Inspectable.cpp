// Fill out your copyright notice in the Description page of Project Settings.


#include "Item_Inspectable.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SpotLightComponent.h"
#include "TimerManager.h"


AItem_Inspectable::AItem_Inspectable()
{

	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 50.0f; 
	SpringArm->bDoCollisionTest = false; 
	SpringArm->bUsePawnControlRotation = false; 

	ItemCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InspectCamera"));
	ItemCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); // что за сокет?
	ItemCamera->PostProcessSettings.bOverride_DepthOfFieldFocalDistance = true;
	ItemCamera->PostProcessSettings.DepthOfFieldFocalDistance = SpringArm->TargetArmLength;
	ItemCamera->PostProcessSettings.bOverride_DepthOfFieldFstop = true;
	ItemCamera->PostProcessSettings.DepthOfFieldFstop = 1.2f; 

	InspectLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("InspectingLight"));
	InspectLight->SetupAttachment(ItemCamera);
	InspectLight->SetVisibility(false);
	InspectLight->AttenuationRadius = 500.0f;
}

void AItem_Inspectable::BeginPlay()
{
	Super::BeginPlay();

	InspectLight->SetIntensity(Config.IntensityLight);

	DefaultState.ArmLength = SpringArm->TargetArmLength;
	DefaultState.Fstop = ItemCamera->PostProcessSettings.DepthOfFieldFstop;
	DefaultState.SpringArmRelativeRotation = SpringArm->GetRelativeRotation();
}


void AItem_Inspectable::Interact(AActor* Interactor, UPrimitiveComponent* HitComponent){}

void AItem_Inspectable::OnFocus(AActor* Interactor, UPrimitiveComponent* HitComponent)
{
	if (CurrentInteractor != nullptr) return;

	if (bEnableOutline && ItemMesh)
	{
		ItemMesh->SetRenderCustomDepth(true);
		ItemMesh->SetCustomDepthStencilValue(255);
	}
}

void AItem_Inspectable::OnLostFocus(AActor* Interactor, UPrimitiveComponent* HitComponent)
{
	if (CurrentInteractor != nullptr) return;
	if (ItemMesh) ItemMesh->SetRenderCustomDepth(false);
}

void AItem_Inspectable::Inspect(AActor* Interactor, UPrimitiveComponent* HitComponent)
{
	if (CurrentInteractor != nullptr)
	{
		return;
	}

	CurrentInteractor = Interactor;

	CurrentInspectRotation = SpringArm->GetRelativeRotation();
	GetWorldTimerManager().ClearTimer(ResetRotationTimerHandle);
	ItemMesh->SetRenderCustomDepth(false);

	if (Config.bAutoRotateMode)
	{
		DefaultState.ActorRotation = GetActorRotation();
		FRotator WorldRot = SpringArm->GetComponentRotation();

		SpringArm->bInheritPitch = false;
		SpringArm->bInheritRoll = false;
		SpringArm->bInheritYaw = false;
		SpringArm->SetRelativeRotation(WorldRot);

		SpringArm->TargetArmLength = Config.ClouseUpArmLength;
		ItemCamera->PostProcessSettings.DepthOfFieldFocalDistance = Config.ClouseUpArmLength;
		ItemCamera->PostProcessSettings.DepthOfFieldMinFstop = 0.4f;
		ItemCamera->PostProcessSettings.bOverride_VignetteIntensity = true;
		ItemCamera->PostProcessSettings.VignetteIntensity = 1.0f;
	}
	
	if (Config.bEnableInspectLight && InspectLight)
	{
		InspectLight->SetVisibility(true);
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC) PC->SetViewTargetWithBlend(this, 1.0f, EViewTargetBlendFunction::VTBlend_Cubic);
}

void AItem_Inspectable::RotateCamera(float PitchInput, float YawInput)
{
	if (!CurrentInteractor) return;

	if (Config.bAutoRotateMode)
	{
		AddActorLocalRotation(FRotator(0.0f, YawInput * 2.0f, 0.0f));
		return;
	}

	CurrentInspectRotation.Yaw += YawInput;
	if (Config.bLimitYaw)
	{
		CurrentInspectRotation.Yaw = FMath::Clamp(CurrentInspectRotation.Yaw, Config.MinYaw, Config.MaxYaw);
	}
	CurrentInspectRotation.Pitch = FMath::Clamp(CurrentInspectRotation.Pitch + PitchInput, Config.MinPitch, Config.MaxPitch);

	SpringArm->SetRelativeRotation(CurrentInspectRotation);
}

void AItem_Inspectable::StopInspect()
{
	CurrentInteractor = nullptr;

	if (InspectLight) InspectLight->SetVisibility(false);
	
	if (Config.bAutoRotateMode) SetActorRotation(DefaultState.ActorRotation);
	
	GetWorldTimerManager().SetTimer(ResetRotationTimerHandle, this, &AItem_Inspectable::ResetCameraRotation, Config.ResetRotationDelay, false);
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("StopInspect ready!"));
}

void AItem_Inspectable::ResetCameraRotation()
{
	if (SpringArm)
	{
		if (Config.bAutoRotateMode)
		{
			SpringArm->bInheritPitch = true;
			SpringArm->bInheritRoll = true;
			SpringArm->bInheritYaw = true;

			SpringArm->TargetArmLength = DefaultState.ArmLength;
			ItemCamera->PostProcessSettings.DepthOfFieldFocalDistance = DefaultState.ArmLength;
			ItemCamera->PostProcessSettings.DepthOfFieldFstop = DefaultState.Fstop;
			ItemCamera->PostProcessSettings.bOverride_VignetteIntensity = false;
		}
		SpringArm->SetRelativeRotation(DefaultState.SpringArmRelativeRotation);
	}
}



