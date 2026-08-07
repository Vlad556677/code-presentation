// Fill out your copyright notice in the Description page of Project Settings.


#include "DetectiveCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Components/BoxComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Main_Interface.h"
#include "GameFramework/SpringArmComponent.h"
#include "EquipmentComponent.h"
#include "DrawDebugHelpers.h"
#include "EvidenceItem.h"

namespace
{
	// input + camera
	constexpr const float MouseRollSensitivity = 2.0f;
	constexpr const float MaxMouseRollAngle = 8.0f;
	constexpr const float MinMouseRollAngle = -8.0f;

	// collision + default = camera
	constexpr float DefaultCapsuleRadius = 40.0f;
	constexpr float DefaultCapsuleHalfHeight = 92.0f;
	constexpr float DefaultSpringArmX = -10.0f;
	constexpr float DefaultSpringArmZ = 60.0f;
	constexpr float DefaultSpringArmLength = 0.0f;

	// setting up the mechanics
	constexpr float TraceTimerInterval = 0.1f;
	constexpr float CameraTransitionTime = 1.0f;
	constexpr float InvertMultiplier = -1.0f;
	constexpr float TargetRollMultiplier = 0.8f;
	constexpr float SprintThresholdY = 0.1f;
	constexpr float TraceDistance = 400.0f;
}

ADetectiveCharacter::ADetectiveCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(DefaultCapsuleRadius, DefaultCapsuleHalfHeight);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpirngArm"));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->SetRelativeLocation(FVector(DefaultSpringArmX, DefaultSpringArmLength, DefaultSpringArmZ));
	SpringArm->TargetArmLength = 0.0f;
	SpringArm->bEnableCameraLag = false;
	SpringArm->bUsePawnControlRotation = true;

	PersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Player"));
	PersonCamera->SetupAttachment(SpringArm);
	PersonCamera->SetRelativeLocation(FVector::ZeroVector);
	PersonCamera->bUsePawnControlRotation = false;

	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("Equipment Component"));

	SightBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SightBox"));
	SightBox->SetupAttachment(PersonCamera);
	SightBox->SetBoxExtent(FVector(400.0f, 10.0f, 10.0f));
	SightBox->SetRelativeLocation(FVector(400.0f, 0.0f, 0.0f));
	SightBox->SetCollisionProfileName(TEXT("Triger"));
	SightBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SightBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	SightBox->SetGenerateOverlapEvents(true);

	TargetRoll = 0.0f;
	MouseRoll = 0.0f;
	CurrentMoveY = 0.0f;
	MouseSensivity = 1.0f;
	WalkSpeed = 400.0f;
	SprintSpeed = 600.0f;
	Stamina = 100.0f;

	bIsTired = false;
	CurrentFocusActor = nullptr;
	CurrentState = EPlayerDeed::FreeWalk;
}


void ADetectiveCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (SightBox)
	{
		SightBox->OnComponentBeginOverlap.AddDynamic(this, &ADetectiveCharacter::OnSightBeginOverlap);
		SightBox->OnComponentEndOverlap.AddDynamic(this, &ADetectiveCharacter::OnSightEndOverlap);
	}
}

//            ===-Player-Input-Controller-===
void ADetectiveCharacter::SetupPlayerInputComponent(UInputComponent* const PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	APlayerController* const PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) return;
	UEnhancedInputLocalPlayerSubsystem* const Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!Subsystem) return;

	Subsystem->ClearAllMappings(); 
	Subsystem->AddMappingContext(InputBaseMapping, 0); 

	if (UEnhancedInputComponent* const Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// ===-Base-Input-Player-===
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADetectiveCharacter::Move_Button_WS);
		Input->BindAction(MoveAction, ETriggerEvent::Completed, this, &ADetectiveCharacter::Move_Completed);

		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADetectiveCharacter::Move_Button_AD);
		Input->BindAction(LookAction, ETriggerEvent::Completed, this, &ADetectiveCharacter::Look_Completed);
		
		Input->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ADetectiveCharacter::Spring_Triggered);
		Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &ADetectiveCharacter::Spring_Completed);
		
		// ===-Interaction-Input-Player-===
		Input->BindAction(Interaction, ETriggerEvent::Started, this, &ADetectiveCharacter::Interaction_Button_E);

		// ===-Diary-===
		Input->BindAction(OpenDiaryAction, ETriggerEvent::Started, this, &ADetectiveCharacter::Open_Diary);
		Input->BindAction(TurnPageAction, ETriggerEvent::Started, this, &ADetectiveCharacter::TurnPage);
	}
}

void ADetectiveCharacter::Move_Button_WS_Implementation(const FInputActionValue& Value)
{
	if (CurrentState != EPlayerDeed::FreeWalk) return;
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDitection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		const float ClampedX = FMath::Clamp(MovementVector.X, -1.0f, 1.0f);
		const float ClampedY = FMath::Clamp(MovementVector.Y, -1.0f, 1.0f);

		AddMovementInput(GetActorRightVector(), ClampedX);
		AddMovementInput(GetActorForwardVector(), ClampedY);

		// tilt the camera when stepping to the side
		TargetRoll = ClampedX * TargetRollMultiplier;

	}
	CurrentMoveY = MovementVector.Y;
}

void ADetectiveCharacter::Move_Button_AD_Implementation(const FInputActionValue& Value)
{
	if (CurrentState == EPlayerDeed::Transitioning) return;
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (CurrentState == EPlayerDeed::Inspecting && InspectableItem)
	{
		InspectableItem->RotateCamera(LookAxisVector.Y * InvertMultiplier, LookAxisVector.X);
	}
	else // the usual review
	{
		const float RollCalculation = LookAxisVector.X * MouseRollSensitivity;
		MouseRoll = FMath::Clamp(RollCalculation, MinMouseRollAngle, MaxMouseRollAngle);

		AddControllerYawInput(LookAxisVector.X * MouseSensivity);
		AddControllerPitchInput(LookAxisVector.Y * MouseSensivity * InvertMultiplier);
	}
}

void ADetectiveCharacter::Interaction_Button_E_Implementation()
{
	if (CurrentState == EPlayerDeed::Transitioning) return;

	//exit from the inspection
	if (CurrentState == EPlayerDeed::Inspecting && InspectableItem)
	{
		APlayerController* const PC = Cast<APlayerController>(GetController());
		if (PC) PC->SetViewTargetWithBlend(this, CameraTransitionTime, EViewTargetBlendFunction::VTBlend_Cubic);
		
		InspectableItem->StopInspect();
		InspectableItem = nullptr;
		CurrentState = EPlayerDeed::Transitioning;

		GetWorldTimerManager().SetTimer(CameraTransitionTimer, this, &ADetectiveCharacter::OnCameraTransitionFinished, CameraTransitionTime, false);
		return;
	}

	// the beginning of interaction with the subject
	if (CurrentFocusActor)
	{
		IMain_Interface* const InteractableActor = Cast<IMain_Interface>(CurrentFocusActor);
		if (!InteractableActor) return;

		// if it is an object that can be viewed
		if (AEvidenceItem* const Item = Cast<AEvidenceItem>(CurrentFocusActor))
		{
			InteractableActor->OnLostFocus(this, nullptr);
			// + Copy thought to clipboard
			BufferLines.Add("a strange cube. Why isn't he black?");
			InteractableActor->Inspect(this, CurrentFocusComponent);
			InspectableItem = Item;
			CurrentState = EPlayerDeed::Transitioning;

			GetWorldTimerManager().SetTimer(CameraTransitionTimer, this, &ADetectiveCharacter::OnCameraTransitionFinished, CameraTransitionTime, false);
		}
		else // ordinary object
		{
			InteractableActor->Interact(this, CurrentFocusComponent);
		}
	}
}

void ADetectiveCharacter::Open_Diary(const FInputActionValue& Value)
{
	APlayerController* const PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) return;

	UEnhancedInputLocalPlayerSubsystem* const Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (Subsystem)
	{
		// get out the diary
		if (CurrentState == EPlayerDeed::FreeWalk)
		{
			CurrentState = EPlayerDeed::ReadingDiary;
			Subsystem->RemoveMappingContext(InputBaseMapping);
			Subsystem->AddMappingContext(InputDiaryMapping, 1);
			if (EquipmentComponent)
			{
				EquipmentComponent->EquipAnItem(DiaryClassToEquip, PersonCamera);
			}
		}
		// put away
		else if (CurrentState == EPlayerDeed::ReadingDiary)
		{
			CurrentState = EPlayerDeed::FreeWalk;
			Subsystem->RemoveMappingContext(InputDiaryMapping);
			Subsystem->AddMappingContext(InputBaseMapping, 0);
			if (EquipmentComponent)
			{
				EquipmentComponent->UnequipCurrentItem();
			}
		}
	}
}

void ADetectiveCharacter::TurnPage(const FInputActionValue& Value)
{	
	const float Number = Value.Get<float>();

	if (EquipmentComponent)
	{
		EquipmentComponent->AdditionsActionItem(Number);
	}
}

void ADetectiveCharacter::Move_Completed_Implementation(const FInputActionValue& Value)
{
	TargetRoll = 0.0f;
	CurrentMoveY = 0.0f;
}

void ADetectiveCharacter::Look_Completed_Implementation(const FInputActionValue& Value)
{
	MouseRoll = 0.0f;
}

void ADetectiveCharacter::Spring_Triggered_Implementation(const FInputActionValue& Value)
{
	if (CurrentMoveY > SprintThresholdY && !bIsTired)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void ADetectiveCharacter::Spring_Completed_Implementation(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ADetectiveCharacter::OnCameraTransitionFinished()
{
	if (InspectableItem)
	{
		CurrentState = EPlayerDeed::Inspecting;
	}
	else
	{
		CurrentState = EPlayerDeed::FreeWalk;
	}
}

void ADetectiveCharacter::OnSightBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// reading about / looking at ourselves
	if (CurrentState != EPlayerDeed::FreeWalk || OtherActor == this || OtherActor->GetOwner() == this) return;

	// Check if we can interact with this object.
	if (OtherActor && OtherActor->GetClass()->ImplementsInterface(UMain_Interface::StaticClass()))
	{
		if (CurrentFocusActor && CurrentFocusActor != OtherActor) // Lost focus if looking at another actor.
		{
			IMain_Interface* const OldInteractable = Cast<IMain_Interface>(CurrentFocusActor);
			if (OldInteractable) OldInteractable->OnLostFocus(this, nullptr);
		}

		// lighting type
		CurrentFocusActor = OtherActor;
		CurrentFocusComponent = OtherComp;

		IMain_Interface* const NewInteractable = Cast<IMain_Interface>(CurrentFocusActor);
		if (NewInteractable)
		{
			NewInteractable->OnFocus(this, CurrentFocusComponent);
		}
	}
}

void ADetectiveCharacter::OnSightEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// remove focus
	if (OtherActor == CurrentFocusActor && OtherComp == CurrentFocusComponent && CurrentFocusActor != nullptr)
	{
		IMain_Interface* const OldInteractable = Cast<IMain_Interface>(CurrentFocusActor);
		if (OldInteractable) OldInteractable->OnLostFocus(this, nullptr);

		CurrentFocusActor = nullptr;
		CurrentFocusComponent = nullptr;
	}
}
