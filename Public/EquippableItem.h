// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EquippableItem.generated.h"

class USceneComponent;
class UTimelineComponent;
class UCurveFloat;
class UWidgetComponent;
class USkeletalMeshComponent;
class UTextBlock;

UCLASS()
class BETA_LIGHTHOUSE_API AEquippableItem : public AActor
{
	GENERATED_BODY()
	
///=======================
///       Ã≈“Œƒ€
///=======================

public:
	AEquippableItem();

	UFUNCTION(BlueprintCallable, Category = "Pages")
	void TurnNextPage();

	UFUNCTION(BlueprintCallable, Category = "Pages")
	void TurnPreviousPage();

	UFUNCTION(BlueprintCallable, Category = "Pages")
	void UpdatePageDisplay();

	UFUNCTION(BlueprintCallable, Category = "Pages")
	void StartWriting(FString NewText);

	UFUNCTION(BlueprintCallable, Category = "Pages")
	void TypeNextCharacter();

	UFUNCTION(BlueprintCallable, Category = "Pages")
	void OnTimelineFinished();

protected:
	UFUNCTION(BlueprintCallable, Category = "Pages")
	void UpdateTimeline(float Alpha);

///=======================
///   VIRTUALññÃ≈“Œƒ€
///=======================

public:
	virtual void Equip(USceneComponent* LabelToAttach);
	virtual void Unequip();
	virtual void MainAction();
	virtual void SecondaryAction(float Number);

protected:
	virtual void BeginPlay() override;

///=======================
///       œŒÀﬂ
///=======================

public:
	// === FString ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pages")
	TArray<FString>DiaryPages;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString FullText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString CurrentText;

	// === int32 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pages")
	int32 CurrentPageIndex = 0;

	// === TextBlock ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextBlock* RightItemText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextBlock* LeftItemText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextBlock* TargetTextForWriting;

	// === bool ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsEquipped;

	// === Components ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTimelineComponent* LiftingItemTimeLine;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* LiftingItemCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* DiaryWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* ItemMesh;

private:
	FTimerHandle NewLetterTimer;
};
