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
#include "DetectiveCharacter.h"

namespace
{
	// height settings
	constexpr const float StartHeightZ = -100.0f;
	constexpr const float FinishHeightZ = -15.0f;

	// texture size
	constexpr float DiaryDrawSize = 1024.0f;

	// Transformation stats (when equipped)
	constexpr float EquipPitch = 15.0f;
	constexpr float EquipYaw = 180.0f;
	constexpr float EquipRoll = 0.0f;

	// displacement of an object
	constexpr float UpdateItemX = 40.0f;
	constexpr float UpdateItemY = 0.0f;

	// print speed
	constexpr float TypeSpeedRate = 0.2f;
}

AEquippableItem::AEquippableItem()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletItemMesh"));
	SetRootComponent(ItemMesh);

	LiftingItemTimeLine = CreateDefaultSubobject<UTimelineComponent>(TEXT("LiftingTimelice"));

	DiaryWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DiaryWidget"));
	DiaryWidgetComponent->SetupAttachment(ItemMesh);
	DiaryWidgetComponent->SetDrawSize(FVector2D(DiaryDrawSize, DiaryDrawSize));
	DiaryWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bIsEquipped = false;
}

void AEquippableItem::BeginPlay()
{
	Super::BeginPlay();

	if (!ItemMesh || !DiaryWidgetComponent || !LiftingItemTimeLine) return;

	// ===- Linking Timeline events -===
	if (LiftingItemCurve)
	{
		FOnTimelineFloat Broker;
		Broker.BindDynamic(this, &AEquippableItem::UpdateTimeline);
		LiftingItemTimeLine->AddInterpFloat(LiftingItemCurve, Broker);

		FOnTimelineEvent FinishedEvent;
		FinishedEvent.BindDynamic(this, &AEquippableItem::OnTimelineFinished);
		LiftingItemTimeLine->SetTimelineFinishedFunc(FinishedEvent);
	}

	// ===- Setting Up a Dynamic Material -===
	// I am creating a dynamic material to map the widget's 2D texture onto the diary's 3D mesh.
	if (UMaterialInterface* const BaseMaterial = ItemMesh->GetMaterial(0))
	{
		if (UMaterialInstanceDynamic* const DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this))
		{
			ItemMesh->SetMaterial(0, DynamicMaterial);
			if (UTextureRenderTarget2D* const WidgetRenderTarget = DiaryWidgetComponent->GetRenderTarget())
			{
				DynamicMaterial->SetTextureParameterValue(FName("WidgetTexture"), WidgetRenderTarget);
			}
		}
	}

	// ===- Initialization of text blocks -===
	DiaryWidgetComponent->InitWidget();
	UUserWidget* const BaseWidget = DiaryWidgetComponent->GetUserWidgetObject();
	if (BaseWidget)
	{
		UWidget* const FindLeftElement = BaseWidget->GetWidgetFromName(TEXT("LeftText"));
		LeftItemText = Cast<UTextBlock>(FindLeftElement);
		UWidget* const FindRightElement = BaseWidget->GetWidgetFromName(TEXT("RightText"));
		RightItemText = Cast<UTextBlock>(FindRightElement);
		
		if (LeftItemText && RightItemText)
		{
			LeftItemText->SetText(FText::FromString(TEXT(""))); //Initially, it is empty.
			RightItemText->SetText(FText::FromString(TEXT(""))); //Initially, it is empty.
		}
	}
}

void AEquippableItem::Equip(USceneComponent* const LabelToAttach)
{
	// Attaching the object to the player's camera + triggering the spawn animation.
	AttachToComponent(LabelToAttach, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeRotation(FRotator(EquipPitch, EquipYaw, EquipRoll));

	SetActorHiddenInGame(false);
	LiftingItemTimeLine->PlayFromStart();
}

void AEquippableItem::Unequip()
{
	// Actor Hidden In Game
	LiftingItemTimeLine->Reverse();
	bIsEquipped = false;
}

void AEquippableItem::MainAction(){}

void AEquippableItem::UpdateTimeline(const float Alpha)
{
	// I smoothly lift the object along a curve.
	const float CurrentHeight_Z = FMath::Lerp(StartHeightZ, FinishHeightZ, Alpha);
	SetActorRelativeLocation(FVector(UpdateItemX, UpdateItemY, CurrentHeight_Z));
}

// ===- Text logic (text layout) -===
void AEquippableItem::StartWriting(const FString NewText)
{
	DiaryPages.Add(NewText);
	const int32 IndexNewPage = DiaryPages.Num() - 1;

	//  determine which page to write on (left or right)
	CurrentPageIndex = IndexNewPage - (IndexNewPage % 2);
	UpdatePageDisplay();
	if (IndexNewPage % 2 == 0)
	{
		TargetTextForWriting = LeftItemText;
	}
	else
	{
		TargetTextForWriting = RightItemText;
	}

	// preparing the buffer for printing
	FullText = NewText;
	CurrentText = TEXT("");
	if (TargetTextForWriting)
	{
		TargetTextForWriting->SetText(FText::FromString(TEXT("")));
	}

	// starting the print timer
	GetWorldTimerManager().SetTimer(NewLetterTimer, this, &AEquippableItem::TypeNextCharacter, TypeSpeedRate, true);
}

void AEquippableItem::TypeNextCharacter()
{
	// i am adding text from FullText to CurrentText character by character.
	if (CurrentText.Len() < FullText.Len())
	{
		const int32 Index = CurrentText.Len();
		const TCHAR NewLetter = FullText[Index];
		CurrentText.AppendChar(NewLetter);
		if (TargetTextForWriting)
		{
			TargetTextForWriting->SetText(FText::FromString(CurrentText));
		}
	}
	else
	{
		// the text has ended.
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
	// printing stops if the player turns the page while the text is being printed
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

void AEquippableItem::SecondaryAction(const float Number)
{
	if (!bIsEquipped) return;

	// page-turning control
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
	// has the timeline returned to the beginning?
	if (LiftingItemTimeLine && FMath::IsNearlyZero(LiftingItemTimeLine->GetPlaybackPosition()))
	{
		SetActorHiddenInGame(true);
		return;
	}
	// otherwise, the timeline at the end serves as a diary right before your eyes.
	bIsEquipped = true;
	
	ADetectiveCharacter* const Player = Cast<ADetectiveCharacter>(GetOwner());
	if (!Player) return;

	// checking for a new diary entry
	if (Player->BufferLines.Num() > 0)
	{
		const FString NewThought = Player->BufferLines[0];
		StartWriting(NewThought);
		Player->BufferLines.RemoveAt(0); // we remove the thought from the buffer.

	}
}

