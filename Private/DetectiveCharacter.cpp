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
	// ввод + камера
	constexpr const float MouseRollSensitivity = 2.0f;
	constexpr const float MaxMouseRollAngle = 8.0f;
	constexpr const float MinMouseRollAngle = -8.0f;

	// коллизия + default полож камера
	constexpr float DefaultCapsuleRadius = 40.0f;
	constexpr float DefaultCapsuleHalfHeight = 92.0f;
	constexpr float DefaultSpringArmX = -10.0f;
	constexpr float DefaultSpringArmZ = 60.0f;
	constexpr float DefaultSpringArmLength = 0.0f;

	// настройка механики 
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

		// наклон камеры при шаге в сторону 
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
	else // обычный обзор
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

	//выход из осмотра
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

	// начало взаимодействия с предметом
	if (CurrentFocusActor)
	{
		IMain_Interface* const InteractableActor = Cast<IMain_Interface>(CurrentFocusActor);
		if (!InteractableActor) return;

		// если предмет который можно осмотреть
		if (AEvidenceItem* const Item = Cast<AEvidenceItem>(CurrentFocusActor))
		{
			// + мысль в буфер
			BufferLines.Add("a strange cube. Why isn't he black?");
			InteractableActor->Inspect(this, nullptr);
			InspectableItem = Item;
			CurrentState = EPlayerDeed::Transitioning;

			GetWorldTimerManager().SetTimer(CameraTransitionTimer, this, &ADetectiveCharacter::OnCameraTransitionFinished, CameraTransitionTime, false);
		}
		else // обычный предмет
		{
			InteractableActor->Interact(this, nullptr);
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
		// достать дневник
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
		// убрать
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
	// читаем / смотрим на себя 
	if (CurrentState != EPlayerDeed::ReadingDiary || OtherActor == this) return;

	// проверка можем ли взаимодействовать с этим предметом 
	if (OtherActor && OtherActor->GetClass()->ImplementsInterface(UMain_Interface::StaticClass()))
	{
		if (CurrentFocusActor && CurrentFocusActor != OtherActor) // снял фокус если смотрел на другой актор
		{
			IMain_Interface* const OldInteractable = Cast<IMain_Interface>(CurrentFocusActor);
			if (OldInteractable) OldInteractable->OnLostFocus(this, nullptr);
		}

		// вкд подсветка 
		CurrentFocusActor = OtherActor;
		IMain_Interface* const NewInteractable = Cast<IMain_Interface>(CurrentFocusActor);
		if (NewInteractable)
		{
			NewInteractable->OnFocus(this, OtherComp);
		}
	}
}

void ADetectiveCharacter::OnSightEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// снимаем фокус
	if (OtherActor == CurrentFocusActor && CurrentFocusActor != nullptr)
	{
		IMain_Interface* const OldInteractable = Cast<IMain_Interface>(CurrentFocusActor);
		if (OldInteractable) OldInteractable->OnLostFocus(this, nullptr);

		CurrentFocusActor = nullptr;
	}
}
