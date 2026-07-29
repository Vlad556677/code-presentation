// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerVehicle.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/StaticMeshComponent.h"
#include "DetectiveCharacter.h"
#include "Components/BoxComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "TimerManager.h"
#include "InputActionValue.h"

namespace
{
	constexpr const float CmPerSecToKmhMultiplier = 0.036f;
	constexpr const float RollInterpDeltaTime = 0.02f;
	constexpr const float RollInterpSpeed = 10.0f;

	const FVector CameraDefaultLocation(34.0f, -21.0f, 46.0f);
	const FVector RulDefaultLocation(64.0f, -21.0f, 36.0f);
	const FVector LeftDoorDefaultLocation(64.0f, -71.0f, 26.0f);
	const FVector WindowSpeedDefaultLocation(64.0f, -21.0f, 46.0f);
	const FVector GasCoverDefaultLocation(64.0f, -21.0f, 46.0f);
	const FVector GasTankTriggerExtent(20.0f, 20.0f, 20.0f);
	const FVector GasHoleDefaultLocation(-159.542668f, 95.979010f, 16.033979f);

	constexpr float SpeedometerTimerInterval = 0.3f; 
	constexpr float SteeringTimerInterval = 0.02f;
	constexpr float InvertMultiplier = -1.0f;

	constexpr float SteeringTolerance = 0.1f;
	constexpr float DoorOpenAngleY = 90.0f;

	constexpr float GasCoverOpenAngleY = -90.0f;
	constexpr float WarningMessageTime = 3.0f;
}

APlayerVehicle::APlayerVehicle()
{
	PrimaryActorTick.bCanEverTick = false;

	CarCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CarCamera->SetupAttachment(GetMesh());
	CarCamera->SetRelativeLocation(CameraDefaultLocation);
	CarCamera->bUsePawnControlRotation = false;
	bUseControllerRotationYaw = false;

	RulMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rul"));
	RulMesh->SetupAttachment(GetMesh());
	RulMesh->SetRelativeLocation(RulDefaultLocation);
	RulMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	LeftDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorLeft"));
	LeftDoorMesh->SetupAttachment(GetMesh());
	LeftDoorMesh->SetRelativeLocation(LeftDoorDefaultLocation);
	LeftDoorMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	WindowSpeedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Speed"));
	WindowSpeedMesh->SetupAttachment(GetMesh());
	WindowSpeedMesh->SetRelativeLocation(WindowSpeedDefaultLocation);
	WindowSpeedMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	GasCoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bak"));
	GasCoverMesh->SetupAttachment(GetMesh());
	GasCoverMesh->SetRelativeLocation(GasCoverDefaultLocation);

	GasTankTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("BakTrigger"));
	GasTankTrigger->SetupAttachment(GasCoverMesh);
	GasTankTrigger->SetBoxExtent(GasTankTriggerExtent);
	GasTankTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	GasTankTrigger->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GasHoleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("GasHoleLocation"));
	GasHoleLocation->SetupAttachment(GetMesh());
	GasHoleLocation->SetRelativeLocation(GasHoleDefaultLocation);

	MaxSpeedForGauge = 200.0f;
	MaxGaugeAngle = 270.0f;
	MaxRulRotation = 65.0f;

	bIsGasCoverOpen = false;
	bIsRefueling = false;
}

void APlayerVehicle::BeginPlay()
{
	Super::BeginPlay();

	VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

	// таймер обновления спидометра
	GetWorldTimerManager().SetTimer(SpeedometrTimerHandle, this, &APlayerVehicle::UpdateSpeedometr, SpeedometerTimerInterval, true);
}

//===-Input-Controller-===
void APlayerVehicle::SetupPlayerInputComponent(UInputComponent* const InputPlayerComponent)
{
	Super::SetupPlayerInputComponent(InputPlayerComponent);

	if (UEnhancedInputComponent* const EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputPlayerComponent))
	{
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &APlayerVehicle::ApplyThrottle);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &APlayerVehicle::ApplySteering);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerVehicle::ApplyLook);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Triggered, this, &APlayerVehicle::HandbrakeHandler);
		EnhancedInputComponent->BindAction(OpenDoorAction, ETriggerEvent::Triggered, this, &APlayerVehicle::ExitVehicle);
	}
} 

//===-Function-Input-===
void APlayerVehicle::ApplyThrottle(const FInputActionValue& Value)
{
	if (!VehicleMovement) return;

	if (bIsRefueling) // блок движение во время заправки
	{
		VehicleMovement->SetThrottleInput(0.0f);
		VehicleMovement->SetBrakeInput(CarSettings.CoastingBrakeFunction);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("UBERY PISTOLET!"));
		return;
	}

	const float ThrottleValue = Value.Get<float>();

	if (ThrottleValue > 0.0f && !bIsRefueling) // W движ в перед
	{
		VehicleMovement->SetThrottleInput(ThrottleValue);
		VehicleMovement->SetBrakeInput(0.0f);
	}
	else if (ThrottleValue < 0.0f && !bIsRefueling) // S скор назад 
	{
		if (IsMovingForward())
		{
			// s работает как тормоз
			VehicleMovement->SetThrottleInput(0.0f);
			VehicleMovement->SetBrakeInput(FMath::Abs(ThrottleValue));
		}
		else
		{
			// s работает как задний ход
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

void APlayerVehicle::ApplySteering(const FInputActionValue& Value)
{
	if (VehicleMovement)
	{
		VehicleMovement->SetSteeringInput(Value.Get<float>());
		if (!GetWorldTimerManager().IsTimerActive(SteeringTimerHandle))
		{
			GetWorldTimerManager().SetTimer(SteeringTimerHandle, this, &APlayerVehicle::UpdateSteeringWheel, SteeringTimerInterval, true);
		}
	}
}

void APlayerVehicle::ApplyLook(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	FRotator CurrentRotation = CarCamera->GetRelativeRotation();

	// ограничение поворота головы 
	CurrentRotation.Yaw = FMath::Clamp(CurrentRotation.Yaw + LookAxisVector.X, -CarSettings.MaxCameraYaw, CarSettings.MaxCameraYaw);
	CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch + (LookAxisVector.Y * InvertMultiplier), -CarSettings.MaxCameraPitch, CarSettings.MaxCameraPitch);

	CurrentRotation.Roll = 0.0f;
	CarCamera->SetRelativeRotation(CurrentRotation);
}

void APlayerVehicle::HandbrakeHandler(const FInputActionValue& Value)
{
	if (!VehicleMovement) return;
	const bool IsPulled = Value.Get<bool>();
	VehicleMovement->SetHandbrakeInput(IsPulled);

	OnHandbrakeChanged.Broadcast(IsPulled);
}

bool APlayerVehicle::IsMovingForward() const
{
	if (!VehicleMovement) return false;
	return VehicleMovement->GetForwardSpeed() > CarSettings.BrakeToReverseThreshold;
}

void APlayerVehicle::ApplyInteractive(const FInputActionValue& Value) //del
{
	LeftDoorMesh->SetRelativeRotation(FRotator(0.0f, DoorOpenAngleY, 0.0f));
}

//===-Timers-===
	//===-Speedometr-===
void APlayerVehicle::UpdateSpeedometr()
{
	if (VehicleMovement && WindowSpeedMesh)
	{
		// перевод скорости в км/ч
		const float CurrentSpeedKMH = FMath::Abs(VehicleMovement->GetForwardSpeed() * CmPerSecToKmhMultiplier);
		// перенос скорости на стрелку
		const float TargetAngle = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, MaxSpeedForGauge),FVector2D(0.0f, MaxGaugeAngle),CurrentSpeedKMH);
		WindowSpeedMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, TargetAngle));
	}
}
	//===-Rul-===
void APlayerVehicle::UpdateSteeringWheel()
{
	if (VehicleMovement && RulMesh)
	{
		const float TargetAngle = VehicleMovement->GetSteeringInput() * MaxRulRotation;
		const FRotator CurrentRot = RulMesh->GetRelativeRotation();
		const float NewRoll = FMath::FInterpTo(CurrentRot.Roll, TargetAngle, RollInterpDeltaTime, RollInterpSpeed);
		
		RulMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, NewRoll));
		if (FMath::IsNearlyEqual(NewRoll, 0.0f, SteeringTolerance) && FMath::IsNearlyZero(VehicleMovement->GetSteeringInput()))
		{
			RulMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
			GetWorldTimerManager().ClearTimer(SteeringTimerHandle);
		}
	}
}

void APlayerVehicle::EnterVehicle(ADetectiveCharacter* const PlayerCharacter)
{
	if (!VehicleMovement) return;
	APlayerController* const PC = Cast<APlayerController>(PlayerCharacter->GetController());
	
	if (PC)
	{
		// прячем игрока, переход в машину
		StoredPlayer = PlayerCharacter;
		StoredPlayer->SetActorHiddenInGame(true);
		StoredPlayer->SetActorEnableCollision(false);
		StoredPlayer->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);

		if (UEnhancedInputLocalPlayerSubsystem* const Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 0);
		}
		PC->Possess(this);
	}
}

void APlayerVehicle::ExitVehicle(const FInputActionValue& Value)
{
	if (!StoredPlayer) return;

	APlayerController* const PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		// переход в игрока
		if (UEnhancedInputLocalPlayerSubsystem* const Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(InputMapping);
		}

			//вычислекние точки спавна игрока 
		StoredPlayer->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		const FVector ExitLocation = GetActorLocation() + (GetActorRightVector() * -150.0f) + FVector(0.0f, 0.0f, 50.0f);
		StoredPlayer->SetActorLocation(ExitLocation);

		StoredPlayer->SetActorHiddenInGame(false);
		StoredPlayer->SetActorEnableCollision(true);

		PC->Possess(StoredPlayer);

		StoredPlayer = nullptr;
	}
}

void APlayerVehicle::ToggleGasCover()
{
	if (bIsRefueling)
	{
		GEngine->AddOnScreenDebugMessage(-1, WarningMessageTime, FColor::Yellow, TEXT("UBERY PISTOLET!"));
		return;
	}

	bIsGasCoverOpen = !bIsGasCoverOpen;

	if (bIsGasCoverOpen)
	{
		GasCoverMesh->SetRelativeRotation(FRotator(0.0f, GasCoverOpenAngleY, 0.0f));
	}
	else if(!bIsRefueling)
	{
		GasCoverMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	}
}

void APlayerVehicle::Interact(AActor* const Interactor, UPrimitiveComponent* const HitComponent)
{
	if (HitComponent == GasTankTrigger)
	{
		ToggleGasCover();
	}
	else
	{
		if (ADetectiveCharacter* const Player = Cast<ADetectiveCharacter>(Interactor))
		{
			EnterVehicle(Player);
		}
	}
}

void APlayerVehicle::OnFocus(AActor* const Interactor, UPrimitiveComponent* const HitComponent){}

void APlayerVehicle::OnLostFocus(AActor* const Interactor, UPrimitiveComponent* const HitComponent){}

void APlayerVehicle::Inspect(AActor* const Interactor, UPrimitiveComponent* const HitComponent){}
