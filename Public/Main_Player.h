// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "Item_Inspectable.h"
#include "Main_Player.generated.h"

class UInputMappingContext;
class UCameraComponent;
class UInputAction;
class USpringArmComponent;
class UEquipmentComponent;

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	FreeWalk, Transitioning, Inspecting, ReadingDiary
};


UCLASS()
class BETA_LIGHTHOUSE_API AMain_Player : public ACharacter
{
	GENERATED_BODY()

public:
	AMain_Player();

	// ===-Diary-===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Diary)
	TArray<FString> BufferLines;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Diary)
	UEquipmentComponent* EquipmentComponent;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category=Diary)
	TSubclassOf<class AEquippableItem> DiaryClassToEquip;

	//===-Camera-===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Camera, meta= (AllowPrivateAccess = "true"))
	UCameraComponent* PersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	//===-Iput-Action-===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta= (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta= (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta= (AllowPrivateAccess = "true"))
	UInputAction* Interaction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta= (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta= (AllowPrivateAccess = "true"))
	UInputAction* OpenDiaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta= (AllowPrivateAccess = "true"))
	UInputAction* TurnPageAction;

	//===-Input-Mapping-===
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category= Inpit_Mapping, meta= (AllowPrivateAccess = "true"))
	UInputMappingContext* InputBaseMapping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category= Inpit_Mapping, meta= (AllowPrivateAccess = "true"))
	UInputMappingContext* InputDiaryMapping;

	UPROPERTY()
	AItem_Inspectable* InspectableItem;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "State")
	EPlayerState CurrentState = EPlayerState::FreeWalk;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "MySettings|Look")
	float TargetRoll = 0.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "MySettings|Look")
	float MouseRoll = 0.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "MySettings|Look")
	float CurrentMoveY = 0.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "MySettings|Look")
	float MouseSensivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MySettings|Movement")
	float WalkSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MySettings|Movement")
	float SprintSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MySettings|Movement")
	float Stamina = 100.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "MySettings|Movement")
	bool bIsTired = false;

	//===-Functions-====
	UFUNCTION(BlueprintNativeEvent, Category = "MySettings|Function")
	void Move_Button_WS(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintNativeEvent, Category = "MySettings|Function")
	void Move_Button_AD(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, Category = "MySettings|Function")
	void Interaction_Button_E();

	UFUNCTION(BlueprintNativeEvent, Category = "MySettings|Function")
	void Move_Completed(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, Category = "MySettings|Function")
	void Look_Completed(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, Category = "MySettings|Function")
	void Spring_Triggered(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, Category = "MySettings|Function")
	void Spring_Completed(const FInputActionValue& Value);

	UFUNCTION()
	void Open_Diary(const FInputActionValue& Value);

	UFUNCTION()
	void TurnPage(const FInputActionValue& Value);

protected:

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	AActor* CurrentFocusActor = nullptr;

	FTimerHandle CameraTransitionTimer;
	FTimerHandle TraceTimerHandle;
	FTimerHandle StaminaTimerHandle;
	
	void OnCameraTransitionFinished();
	void TraceForInteractables();
};
