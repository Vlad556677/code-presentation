// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DetectiveCharacter.generated.h"

class AEvidenceItem;
class UInputMappingContext;
class UCameraComponent;
class UInputAction;
class USpringArmComponent;
class UEquipmentComponent;

UENUM(BlueprintType)
enum class EPlayerDeed : uint8
{
	FreeWalk, Transitioning, Inspecting, ReadingDiary
};

UCLASS()
class BETA_LIGHTHOUSE_API ADetectiveCharacter : public ACharacter
{
	GENERATED_BODY()

///=======================
///       Ã≈“Œƒ€
///=======================

public:
	ADetectiveCharacter();

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
	UFUNCTION()
	void OnSightBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSightEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	void OnCameraTransitionFinished();
///=======================
///   VIRTUALññÃ≈“Œƒ€
///=======================

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

///=======================
///       œŒÀﬂ
///=======================

public:
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UBoxComponent* SightBox;

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
	AEvidenceItem* InspectableItem;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "State")
	EPlayerDeed CurrentState;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "MySettings|Look")
	float TargetRoll;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "MySettings|Look")
	float MouseRoll;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "MySettings|Look")
	float CurrentMoveY;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "MySettings|Look")
	float MouseSensivity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MySettings|Movement")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MySettings|Movement")
	float SprintSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MySettings|Movement")
	float Stamina;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "MySettings|Movement")
	bool bIsTired;

private:
	AActor* CurrentFocusActor;

	FTimerHandle CameraTransitionTimer;
	FTimerHandle StaminaTimerHandle;
};
