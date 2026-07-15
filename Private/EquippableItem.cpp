// Fill out your copyright notice in the Description page of Project Settings.


#include "EquippableItem.h"
#include "Components/SceneComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Main_Player.h"


AEquippableItem::AEquippableItem()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletItemMesh"));
	SetRootComponent(ItemMesh);

	LiftingItemTimeLine = CreateDefaultSubobject<UTimelineComponent>(TEXT("LiftingTimelice"));

	DiaryWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DiaryWidget"));
	DiaryWidgetComponent->SetupAttachment(ItemMesh);
	DiaryWidgetComponent->SetDrawSize(FVector2D(1024.0f, 1024.0f));
	DiaryWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AEquippableItem::BeginPlay()
{
	Super::BeginPlay();

	if (LiftingItemCurve)
	{
		FOnTimelineFloat Broker;
		Broker.BindDynamic(this, &AEquippableItem::UpdateTimeline);
		LiftingItemTimeLine->AddInterpFloat(LiftingItemCurve, Broker);

		FOnTimelineEvent FinishedEvent;
		FinishedEvent.BindDynamic(this, &AEquippableItem::OnTimelineFinished);
		LiftingItemTimeLine->SetTimelineFinishedFunc(FinishedEvent);
	}
	UMaterialInterface* BaseMaterial = ItemMesh->GetMaterial(0);
	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		if (DynamicMaterial)
		{
			ItemMesh->SetMaterial(0, DynamicMaterial);
			UTextureRenderTarget2D* WidgetRenderTarget = DiaryWidgetComponent->GetRenderTarget();
			if (WidgetRenderTarget)
			{
				DynamicMaterial->SetTextureParameterValue(FName("WidgetTexture"), WidgetRenderTarget);
			}
		}
	}

	///////////////////////
	DiaryWidgetComponent->InitWidget();
	UUserWidget* BaseWidget = DiaryWidgetComponent->GetUserWidgetObject();
	if (BaseWidget)
	{
		UWidget* FindLeftElement = BaseWidget->GetWidgetFromName(TEXT("LeftText"));
		LeftItemText = Cast<UTextBlock>(FindLeftElement);
		UWidget* FindRightElement = BaseWidget->GetWidgetFromName(TEXT("RightText"));
		RightItemText = Cast<UTextBlock>(FindRightElement);
		
		if (LeftItemText && RightItemText)
		{
			LeftItemText->SetText(FText::FromString(TEXT(""))); //Debug
			RightItemText->SetText(FText::FromString(TEXT(""))); //Debug 
		}
		
	}
}

void AEquippableItem::Equip(USceneComponent* LabelToAttach)
{
	this->AttachToComponent(LabelToAttach, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeRotation(FRotator(15.0f, 180.0f, 0.0f));

	SetActorHiddenInGame(false);
	LiftingItemTimeLine->PlayFromStart();
}

void AEquippableItem::Unequip()
{
	LiftingItemTimeLine->Reverse();
	bIsEquipped = false;
}

void AEquippableItem::MainAction()
{
}

void AEquippableItem::UpdateTimeline(float Alpha)
{
	float StartHeight_Z = -100.0f;
	float FinishHeight_Z = -15.0f;

	float CurrentHeight_Z = FMath::Lerp(StartHeight_Z, FinishHeight_Z, Alpha);

	SetActorRelativeLocation(FVector(40.0f, 0.0f, CurrentHeight_Z));


}


void AEquippableItem::StartWriting(FString NewText)
{
	DiaryPages.Add(NewText);
	int32 IndexNewPage = DiaryPages.Num() - 1;

	CurrentPageIndex = IndexNewPage - (IndexNewPage % 2);
	UpdatePageDisplay();
	if (IndexNewPage % 2==0)
	{
		TargetTextForWriting = LeftItemText;
	}
	else
	{
		TargetTextForWriting = RightItemText;
	}

	FullText = NewText;
	CurrentText = TEXT("");
	if (TargetTextForWriting)
	{
		TargetTextForWriting->SetText(FText::FromString(TEXT("")));
	}

	GetWorldTimerManager().SetTimer(NewLetterTimer, this, &AEquippableItem::TypeNextCharacter, 0.2f, true);
}

void AEquippableItem::TypeNextCharacter()
{
	if (CurrentText.Len() < FullText.Len())
	{
		int32 Index = CurrentText.Len();
		TCHAR NewLetter = FullText[Index];
		CurrentText.AppendChar(NewLetter);
		if (TargetTextForWriting)
		{
			TargetTextForWriting->SetText(FText::FromString(CurrentText));
		}
	}
	else
	{
		GetWorldTimerManager().ClearTimer(NewLetterTimer);
	}

}

void AEquippableItem::TurnNextPage()
{
	if (CurrentPageIndex +2 < DiaryPages.Num())
	{
		CurrentPageIndex +=2;
		UpdatePageDisplay();
	}
}

void AEquippableItem::TurnPreviousPage()
{
	if (CurrentPageIndex > 0)
	{
		CurrentPageIndex-=2;
		UpdatePageDisplay();
	}
}

void AEquippableItem::UpdatePageDisplay()
{
	GetWorldTimerManager().ClearTimer(NewLetterTimer);
	// ===-Left-Page-===
	if (LeftItemText && DiaryPages.IsValidIndex(CurrentPageIndex))
	{
		LeftItemText->SetText(FText::FromString(DiaryPages[CurrentPageIndex]));
	}
	else
	{
		LeftItemText->SetText(FText::FromString(TEXT("")));
	}

	// ===-Right-Page-===
	if (RightItemText && DiaryPages.IsValidIndex(CurrentPageIndex +1))
	{
		RightItemText->SetText(FText::FromString(DiaryPages[CurrentPageIndex+1]));
	}
	else
	{
		RightItemText->SetText(FText::FromString(TEXT("")));
	}
}


void AEquippableItem::SecondaryAction(float Number)
{
	if (bIsEquipped==false)
	{
		return;
	}

	if (Number > 0)
	{
		TurnNextPage();
	}
	else if (Number < 0)
	{
		TurnPreviousPage();
	}
}

void AEquippableItem::OnTimelineFinished()
{
	if (LiftingItemTimeLine->GetPlaybackPosition() >= LiftingItemTimeLine->GetTimelineLength())
	{
		bIsEquipped = true;

		AMain_Player* Player = Cast<AMain_Player>(GetOwner());
		if (!Player) return;

		if (Player->BufferLines.Num() > 0)
		{
			FString NewThought = Player->BufferLines[0];
			StartWriting(NewThought);
			Player->BufferLines.RemoveAt(0);

		}
		else if (LiftingItemTimeLine->GetPlaybackPosition() == 0)
		{
			SetActorHiddenInGame(true);
		}
	}
	else if (LiftingItemTimeLine->GetPlaybackPosition() <= 0.0f)
	{
		SetActorHiddenInGame(true);
	}
}

