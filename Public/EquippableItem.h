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
	
public:	
	
	AEquippableItem();
	virtual void Equip(USceneComponent* LabelToAttach);
	virtual void Unequip();
	virtual void MainAction();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTimelineComponent* LiftingItemTimeLine;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* LiftingItemCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* DiaryWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextBlock* RightItemText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextBlock* LeftItemText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextBlock* TargetTextForWriting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString CurrentText;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString FullText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsEquipped = false;

	// ===-Logic-Diary-Pages-===

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Pages")
	TArray<FString>DiaryPages;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Pages")
	int32 CurrentPageIndex = 0;

	UFUNCTION(BlueprintCallable,Category="Pages")
	void TurnNextPage();

	UFUNCTION(BlueprintCallable, Category = "Pages")
	void TurnPreviousPage();

	UFUNCTION(BlueprintCallable, Category = "Pages")
	void UpdatePageDisplay();

	UFUNCTION()
	void StartWriting(FString NewText);

	UFUNCTION()
	void TypeNextCharacter();

	UFUNCTION()
	virtual void SecondaryAction(float Number);

	UFUNCTION()
	void OnTimelineFinished();

	FTimerHandle NewLetterTimer;

protected:
	UFUNCTION()
	void UpdateTimeline(float Alpha);

	virtual void BeginPlay() override;


};
