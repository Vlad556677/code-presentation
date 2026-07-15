// Fill out your copyright notice in the Description page of Project Settings.


#include "Main_Player.h"
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

AMain_Player::AMain_Player()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(40.0f, 92.0f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpirngArm"));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->SetRelativeLocation(FVector(-10.0f, 0.0f, 60.0f));
	SpringArm->TargetArmLength = 0.0f;
	SpringArm->bEnableCameraLag = false;
	SpringArm->bUsePawnControlRotation = true;

	PersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Player"));
	PersonCamera->SetupAttachment(SpringArm);
	PersonCamera->SetRelativeLocation(FVector::ZeroVector);
	PersonCamera->bUsePawnControlRotation = false;

	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("Equipment Component"));

}


void AMain_Player::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(TraceTimerHandle, this, &AMain_Player::TraceForInteractables, 0.1f, true);
}

//            ===-Player-Input-Controller-===
void AMain_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	Subsystem->ClearAllMappings(); // под вопросом нужды
	Subsystem->AddMappingContext(InputBaseMapping, 0); // под вопросом нужды

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// ===-Base-Input-Player-===
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMain_Player::Move_Button_WS);
		Input->BindAction(MoveAction, ETriggerEvent::Completed, this, &AMain_Player::Move_Completed);

		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMain_Player::Move_Button_AD);
		Input->BindAction(LookAction, ETriggerEvent::Completed, this, &AMain_Player::Look_Completed);
		
		Input->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AMain_Player::Spring_Triggered);
		Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &AMain_Player::Spring_Completed);
		
		// ===-Interaction-Input-Player-===
		Input->BindAction(Interaction, ETriggerEvent::Started, this, &AMain_Player::Interaction_Button_E);

		// ===-Diary-===
		Input->BindAction(OpenDiaryAction, ETriggerEvent::Started, this, &AMain_Player::Open_Diary);
		Input->BindAction(TurnPageAction, ETriggerEvent::Started, this, &AMain_Player::TurnPage);
	}
}


void AMain_Player::Move_Button_WS_Implementation(const FInputActionValue& Value)
{
	if (CurrentState != EPlayerState::FreeWalk) return;
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDitection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		float ClampedX = FMath::Clamp(MovementVector.X, -1.0f, 1.0f);
		float ClampedY = FMath::Clamp(MovementVector.Y, -1.0f, 1.0f);

		AddMovementInput(GetActorRightVector(), ClampedX);
		AddMovementInput(GetActorForwardVector(), ClampedY);

		TargetRoll = ClampedX * 0.8f;

	}
	CurrentMoveY = MovementVector.Y;
}

void AMain_Player::Move_Button_AD_Implementation(const FInputActionValue& Value)
{
	if (CurrentState == EPlayerState::Transitioning) return;
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (CurrentState == EPlayerState::Inspecting && InspectableItem)
	{
		InspectableItem->RotateCamera(LookAxisVector.Y * -1, LookAxisVector.X);
	}
	else
	{
		float RollCalculation = LookAxisVector.X * 2.0f;
		MouseRoll = FMath::Clamp(RollCalculation, -8.0f, 8.0f);


		AddControllerYawInput(LookAxisVector.X * MouseSensivity);
		AddControllerPitchInput(LookAxisVector.Y * MouseSensivity * -1.0f);
	}
}

void AMain_Player::Interaction_Button_E_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Butten E"), (int32)CurrentState));

	if (CurrentState == EPlayerState::Transitioning) return;

	//выход из осмотра
	if (CurrentState == EPlayerState::Inspecting && InspectableItem)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC) PC->SetViewTargetWithBlend(this, 1.0f, EViewTargetBlendFunction::VTBlend_Cubic);
		
		InspectableItem->StopInspect();
		InspectableItem = nullptr;
		CurrentState = EPlayerState::Transitioning;

		GetWorldTimerManager().SetTimer(CameraTransitionTimer, this, &AMain_Player::OnCameraTransitionFinished, 1.0f, false);
		return;
	}

	if (CurrentFocusActor)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("CurrentFocusActor ptr!"));
		IMain_Interface* InteractableActor = Cast<IMain_Interface>(CurrentFocusActor);
		if (!InteractableActor) return;

		if (AItem_Inspectable* Item = Cast<AItem_Inspectable>(CurrentFocusActor))
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Ready Inspect"));
			BufferLines.Add("a strange cube. Why isn't he black?");
			InteractableActor->Inspect(this, nullptr);
			InspectableItem = Item;
			CurrentState = EPlayerState::Transitioning;

			GetWorldTimerManager().SetTimer(CameraTransitionTimer, this, &AMain_Player::OnCameraTransitionFinished, 1.0f, false);
		}
		else
		{
			InteractableActor->Interact(this, nullptr);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("CurrentFocusActor emptiness!"));
	}
}

void AMain_Player::Open_Diary(const FInputActionValue& Value)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (Subsystem)
	{
		if (CurrentState == EPlayerState::FreeWalk)
		{
			CurrentState = EPlayerState::ReadingDiary;
			Subsystem->RemoveMappingContext(InputBaseMapping);
			Subsystem->AddMappingContext(InputDiaryMapping, 1);
			if (EquipmentComponent)
			{
				EquipmentComponent->EquipAnItem(DiaryClassToEquip, PersonCamera);
			}
		}
		else if (CurrentState == EPlayerState::ReadingDiary)
		{
			CurrentState = EPlayerState::FreeWalk;
			Subsystem->RemoveMappingContext(InputDiaryMapping);
			Subsystem->AddMappingContext(InputBaseMapping, 0);
			if (EquipmentComponent)
			{
				EquipmentComponent->UnequipCurrentItem();
			}
		}


	}
}

void AMain_Player::TurnPage(const FInputActionValue& Value)
{	
	float Number = Value.Get<float>();

	if (EquipmentComponent)
	{
		EquipmentComponent->AdditionsActionItem(Number);
	}
}


void AMain_Player::Move_Completed_Implementation(const FInputActionValue& Value)
{
	TargetRoll = 0.0f;
	CurrentMoveY = 0.0f;
}

void AMain_Player::Look_Completed_Implementation(const FInputActionValue& Value)
{
	MouseRoll = 0.0f;
}

void AMain_Player::Spring_Triggered_Implementation(const FInputActionValue& Value)
{
	

	if (CurrentMoveY > 0.1f && !bIsTired)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void AMain_Player::Spring_Completed_Implementation(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AMain_Player::OnCameraTransitionFinished()
{
	if (InspectableItem)
	{
		CurrentState = EPlayerState::Inspecting;
	}
	else
	{
		CurrentState = EPlayerState::FreeWalk;
	}
}

void AMain_Player::TraceForInteractables()
{
	if (CurrentState != EPlayerState::FreeWalk || !PersonCamera) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	FVector Start;
	FRotator Rotation;

	PC->GetPlayerViewPoint(Start, Rotation);
	FVector End = Start + (Rotation.Vector() * 800.0f);

	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 3.0f, 0, 2.0f);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams))
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s"), *HitActor->GetName()));
			}
		}


		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		AActor* HitActor = HitResult.GetActor();

		if (HitActor && HitActor->GetClass()->ImplementsInterface(UMain_Interface::StaticClass()))
		{
			if (HitActor != CurrentFocusActor)
			{
				if (CurrentFocusActor)
				{
					IMain_Interface* OldInteractable = Cast<IMain_Interface>(CurrentFocusActor);
					if (OldInteractable) OldInteractable->OnLostFocus(this, nullptr);
				}
			}
			CurrentFocusActor = HitActor;
			IMain_Interface* NewInteractable = Cast<IMain_Interface>(CurrentFocusActor);
			if (NewInteractable)
			{
				NewInteractable->OnFocus(this, HitComponent);
				return;
			}
		}
	}
	if (CurrentFocusActor)
	{
		IMain_Interface* OldInteractable = Cast<IMain_Interface>(CurrentFocusActor);
		if (OldInteractable) OldInteractable->OnLostFocus(this, nullptr);
		CurrentFocusActor = nullptr;
	}
}

