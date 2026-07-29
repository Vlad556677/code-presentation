// Fill out your copyright notice in the Description page of Project Settings.


#include "EvidenceItem.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SpotLightComponent.h"
#include "TimerManager.h"

namespace
{
	constexpr float DefaultArmLength = 50.0f;
	constexpr float DefaultDepthOfFieldFstop = 1.2f;
	constexpr float LightAttenuationRadius = 500.0f;
	constexpr float CloseUpMinFstop = 0.4f;
	constexpr float CloseUpVignetteIntensity = 1.0f;
	constexpr float BlendViewTargetTime = 1.0f;
	constexpr float RotationSensitivity = 2.0f;
	constexpr uint8 CustomDepthStencilOutlineValue = 255;
}

AEvidenceItem::AEvidenceItem()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = DefaultArmLength;
	SpringArm->bDoCollisionTest = false; 
	SpringArm->bUsePawnControlRotation = false; 

	ItemCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InspectCamera"));
	ItemCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); 
	ItemCamera->PostProcessSettings.bOverride_DepthOfFieldFocalDistance = true;
	ItemCamera->PostProcessSettings.DepthOfFieldFocalDistance = SpringArm->TargetArmLength;
	ItemCamera->PostProcessSettings.bOverride_DepthOfFieldFstop = true;
	ItemCamera->PostProcessSettings.DepthOfFieldFstop = DefaultDepthOfFieldFstop;

	InspectLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("InspectingLight"));
	InspectLight->SetupAttachment(ItemCamera);
	InspectLight->SetVisibility(false);
	InspectLight->AttenuationRadius = LightAttenuationRadius;

	bEnableOutline = true;
	CurrentInteractor = nullptr;
}

void AEvidenceItem::BeginPlay()
{
	Super::BeginPlay();

	InspectLight->SetIntensity(Config.IntensityLight);

	// кеширую настройки камеры, чтобы вернуть их после осмотра
	DefaultState.ArmLength = SpringArm->TargetArmLength;
	DefaultState.Fstop = ItemCamera->PostProcessSettings.DepthOfFieldFstop;
	DefaultState.SpringArmRelativeRotation = SpringArm->GetRelativeRotation();
}

void AEvidenceItem::Interact(AActor* const Interactor, UPrimitiveComponent* const HitComponent){}

// вкл подсветки
void AEvidenceItem::OnFocus(AActor* const Interactor, UPrimitiveComponent* const HitComponent)
{
	if (CurrentInteractor != nullptr) return;

	if (bEnableOutline && ItemMesh)
	{
		ItemMesh->SetRenderCustomDepth(true);
		ItemMesh->SetCustomDepthStencilValue(CustomDepthStencilOutlineValue);
	}
}

// выкл подсветки
void AEvidenceItem::OnLostFocus(AActor* const Interactor, UPrimitiveComponent* const HitComponent)
{
	if (CurrentInteractor != nullptr) return;
	if (ItemMesh) ItemMesh->SetRenderCustomDepth(false);
}

void AEvidenceItem::Inspect(AActor* const Interactor, UPrimitiveComponent* const HitComponent)
{
	if (CurrentInteractor != nullptr) return;

	CurrentInteractor = Interactor;

	CurrentInspectRotation = SpringArm->GetRelativeRotation();
	GetWorldTimerManager().ClearTimer(ResetRotationTimerHandle);
	ItemMesh->SetRenderCustomDepth(false);

	// автовращение предмета 
	if (Config.bAutoRotateMode)
	{
		DefaultState.ActorRotation = GetActorRotation();
		FRotator const WorldRot = SpringArm->GetComponentRotation();

		SpringArm->bInheritPitch = false;
		SpringArm->bInheritRoll = false;
		SpringArm->bInheritYaw = false;
		SpringArm->SetRelativeRotation(WorldRot);

		SpringArm->TargetArmLength = Config.ClouseUpArmLength;
		ItemCamera->PostProcessSettings.DepthOfFieldFocalDistance = Config.ClouseUpArmLength;
		ItemCamera->PostProcessSettings.DepthOfFieldMinFstop = CloseUpMinFstop;
		ItemCamera->PostProcessSettings.bOverride_VignetteIntensity = true;
		ItemCamera->PostProcessSettings.VignetteIntensity = CloseUpVignetteIntensity;
	}
	
	if (Config.bEnableInspectLight && InspectLight)
	{
		InspectLight->SetVisibility(true);
	}

	APlayerController* const PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC) PC->SetViewTargetWithBlend(this, BlendViewTargetTime, EViewTargetBlendFunction::VTBlend_Cubic);
}

void AEvidenceItem::RotateCamera(const float PitchInput, const float YawInput)
{
	if (!CurrentInteractor) return;

	// автовращение 
	if (Config.bAutoRotateMode)
	{
		AddActorLocalRotation(FRotator(0.0f, YawInput * RotationSensitivity, 0.0f));
		return;
	}

	// вращаем камеру вокруг предмета
	CurrentInspectRotation.Yaw += YawInput;
	if (Config.bLimitYaw)
	{
		CurrentInspectRotation.Yaw = FMath::Clamp(CurrentInspectRotation.Yaw, Config.MinYaw, Config.MaxYaw);
	}
	CurrentInspectRotation.Pitch = FMath::Clamp(CurrentInspectRotation.Pitch + PitchInput, Config.MinPitch, Config.MaxPitch);

	SpringArm->SetRelativeRotation(CurrentInspectRotation);
}

void AEvidenceItem::StopInspect()
{
	CurrentInteractor = nullptr;

	if (InspectLight) InspectLight->SetVisibility(false);
	
	if (Config.bAutoRotateMode) SetActorRotation(DefaultState.ActorRotation);
	
	GetWorldTimerManager().SetTimer(ResetRotationTimerHandle, this, &AEvidenceItem::ResetCameraRotation, Config.ResetRotationDelay, false);
}

void AEvidenceItem::ResetCameraRotation()
{
	// возвращение предыщуних параметров 
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