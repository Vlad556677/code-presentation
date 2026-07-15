// Fill out your copyright notice in the Description page of Project Settings.


#include "VWP_PlayerCar.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/StaticMeshComponent.h"
#include "Main_Player.h"
#include "Components/BoxComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "TimerManager.h"
#include "InputActionValue.h"


AVWP_PlayerCar::AVWP_PlayerCar()
{
	PrimaryActorTick.bCanEverTick = false;

	CarCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CarCamera->SetupAttachment(GetMesh());
	CarCamera->SetRelativeLocation(FVector(34.0f, -21.0f, 46.0f));
	CarCamera->bUsePawnControlRotation = false;
	bUseControllerRotationYaw = false;

	
	RulMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rul"));
	RulMesh->SetupAttachment(GetMesh());
	RulMesh->SetRelativeLocation(FVector(64.0f, -21.0f, 36.0f));
	RulMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	LeftDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorLeft"));
	LeftDoorMesh->SetupAttachment(GetMesh());
	LeftDoorMesh->SetRelativeLocation(FVector(64.0f, -71.0f, 26.0f));
	LeftDoorMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	WindowSpeedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Speed"));
	WindowSpeedMesh->SetupAttachment(GetMesh());
	WindowSpeedMesh->SetRelativeLocation(FVector(64.0f, -21.0f, 46.0f));
	WindowSpeedMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	GasCoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bak"));
	GasCoverMesh->SetupAttachment(GetMesh());
	GasCoverMesh->SetRelativeLocation(FVector(64.0f, -21.0f, 46.0f));

	GasTankTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("BakTrigger"));
	GasTankTrigger->SetupAttachment(GasCoverMesh);
	GasTankTrigger->SetBoxExtent(FVector(20.0F, 20.0F, 20.0F));
	GasTankTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	GasTankTrigger->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GasHoleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("GasHoleLocation"));
	GasHoleLocation->SetupAttachment(GetMesh());
	GasHoleLocation->SetRelativeLocation(FVector(-159.542668, 95.979010, 16.033979));
}


void AVWP_PlayerCar::BeginPlay()
{
	Super::BeginPlay();

	VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

	GetWorldTimerManager().SetTimer(SpeedometrTimerHandle, this, &AVWP_PlayerCar::UpdateSpeedometr, 0.3f, true);
}

//===-Input-Controller-===
void AVWP_PlayerCar::SetupPlayerInputComponent(UInputComponent* InputPlayerComponent)
{
	Super::SetupPlayerInputComponent(InputPlayerComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputPlayerComponent))
	{
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AVWP_PlayerCar::ApplyThrottle);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AVWP_PlayerCar::ApplySteering);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AVWP_PlayerCar::ApplyLook);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Triggered, this, &AVWP_PlayerCar::HandbrakeHandler);
		EnhancedInputComponent->BindAction(OpenDoorAction, ETriggerEvent::Triggered, this, &AVWP_PlayerCar::ExitVehicle);
	}
} 

//===-Function-Input-===
void AVWP_PlayerCar::ApplyThrottle(const FInputActionValue& Value)
{
	if (!VehicleMovement) return;

	if (bIsRefueling)
	{
		VehicleMovement->SetThrottleInput(0.0f);
		VehicleMovement->SetBrakeInput(CarSettings.CoastingBrakeFunction);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("UBERY PISTOLET!"));
		return;
	}

	float ThrottleValue = Value.Get<float>();

	if (ThrottleValue > 0.0f && !bIsRefueling) // W движ в перед
	{
		VehicleMovement->SetThrottleInput(ThrottleValue);
		VehicleMovement->SetBrakeInput(0.0f);
	}
	else if (ThrottleValue < 0.0f && !bIsRefueling) // S скор назад 
	{
		if (IsMovingForward())
		{
			VehicleMovement->SetThrottleInput(0.0f);
			VehicleMovement->SetBrakeInput(FMath::Abs(ThrottleValue));
		}
		else
		{
			VehicleMovement->SetThrottleInput(ThrottleValue); 
			VehicleMovement->SetBrakeInput(0.0f);
		}
	}
	else // нет ввода 
	{
		VehicleMovement->SetThrottleInput(0.0f);
		VehicleMovement->SetBrakeInput(CarSettings.CoastingBrakeFunction);
	}
}
void AVWP_PlayerCar::ApplySteering(const FInputActionValue& Value)
{
	if (VehicleMovement)
	{
		VehicleMovement->SetSteeringInput(Value.Get<float>());
		if (!GetWorldTimerManager().IsTimerActive(SteeringTimerHandle))
		{
			GetWorldTimerManager().SetTimer(SteeringTimerHandle, this, &AVWP_PlayerCar::UpdateSteeringWheel, 0.02f, true);
		}
	}
}
void AVWP_PlayerCar::ApplyLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	FRotator CurrentRotation = CarCamera->GetRelativeRotation();

	CurrentRotation.Yaw = FMath::Clamp(CurrentRotation.Yaw + LookAxisVector.X, -CarSettings.MaxCameraYaw, CarSettings.MaxCameraYaw);
	CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch + (-LookAxisVector.Y), -CarSettings.MaxCameraPitch, CarSettings.MaxCameraPitch);

	CurrentRotation.Roll = 0.0f;
	CarCamera->SetRelativeRotation(CurrentRotation);
}
void AVWP_PlayerCar::HandbrakeHandler(const FInputActionValue& Value)
{
	if (!VehicleMovement) return;
	bool IsPulled = Value.Get<bool>();
	VehicleMovement->SetHandbrakeInput(IsPulled);

	OnHandbrakeChanged.Broadcast(IsPulled);
}
bool AVWP_PlayerCar::IsMovingForward() const
{
	if (!VehicleMovement) return false;
	return VehicleMovement->GetForwardSpeed() > CarSettings.BrakeToReverseThreshold;
}

void AVWP_PlayerCar::ApplyInteractive(const FInputActionValue& Value) //del
{
	LeftDoorMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
}

//===-Timers-===
	//===-Speedometr-===
void AVWP_PlayerCar::UpdateSpeedometr()
{
	if (VehicleMovement && WindowSpeedMesh)
	{
		float CurrentSpeedKMH = FMath::Abs(VehicleMovement->GetForwardSpeed() * 0.036f);


		float TargetAngle = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, MaxSpeedForGauge),FVector2D(0.0f, MaxGaugeAngle),CurrentSpeedKMH);
		WindowSpeedMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, TargetAngle));
	}
}
	//===-Rul-===
void AVWP_PlayerCar::UpdateSteeringWheel()
{
	if (VehicleMovement && RulMesh)
	{
		float TargetAngle = VehicleMovement->GetSteeringInput() * MaxRulRotation;

		FRotator CurrentRot = RulMesh->GetRelativeRotation();

		float NewRoll = FMath::FInterpTo(CurrentRot.Roll, TargetAngle, 0.02f, 10.0f);
		RulMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, NewRoll));
		if (FMath::IsNearlyEqual(NewRoll, 0.0f, 0.1f) && FMath::IsNearlyZero(VehicleMovement->GetSteeringInput()))
		{
			RulMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
			GetWorldTimerManager().ClearTimer(SteeringTimerHandle);
		}
	}
}

void AVWP_PlayerCar::EnterVehicle(AMain_Player* PlayerCharacter)
{
	if (!VehicleMovement) return;

	APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());
	if (PC)
	{
		StoredPlayer = PlayerCharacter;
		StoredPlayer->SetActorHiddenInGame(true);
		StoredPlayer->SetActorEnableCollision(false);
		StoredPlayer->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 0);
		}
		PC->Possess(this);
	}
}

void AVWP_PlayerCar::ExitVehicle(const FInputActionValue& Value)
{
	if (!StoredPlayer) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(InputMapping);
		}
		StoredPlayer->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		FVector ExitLocation = GetActorLocation() + (GetActorRightVector() * -150.0f) + FVector(0.0f, 0.0f, 50.0f);
		StoredPlayer->SetActorLocation(ExitLocation);

		StoredPlayer->SetActorHiddenInGame(false);
		StoredPlayer->SetActorEnableCollision(true);

		PC->Possess(StoredPlayer);

		StoredPlayer = nullptr;
	}
}


void AVWP_PlayerCar::ToggleGasCover()
{
	if (bIsRefueling)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("UBERY PISTOLET!"));
		return;
	}

	bIsGasCoverOpen = !bIsGasCoverOpen;

	if (bIsGasCoverOpen)
	{
		GasCoverMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	}
	else if(!bIsRefueling)
	{
		GasCoverMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	}
}

void AVWP_PlayerCar::Interact(AActor* Interactor, UPrimitiveComponent* HitComponent)
{
	if (HitComponent == GasTankTrigger)
	{
		ToggleGasCover();
	}
	else
	{
		if (AMain_Player* Player = Cast<AMain_Player>(Interactor))
		{
			EnterVehicle(Player);
		}
	}
}

void AVWP_PlayerCar::OnFocus(AActor* Interactor, UPrimitiveComponent* HitComponent){}

void AVWP_PlayerCar::OnLostFocus(AActor* Interactor, UPrimitiveComponent* HitComponent){}

void AVWP_PlayerCar::Inspect(AActor* Interactor, UPrimitiveComponent* HitComponent){}
