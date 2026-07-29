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
	// настройки высоты
	constexpr const float StartHeightZ = -100.0f;
	constexpr const float FinishHeightZ = -15.0f;

	// размер текстуры
	constexpr float DiaryDrawSize = 1024.0f;

	// параметры трансформации( при экипировке)
	constexpr float EquipPitch = 15.0f;
	constexpr float EquipYaw = 180.0f;
	constexpr float EquipRoll = 0.0f;

	// смещение предмета
	constexpr float UpdateItemX = 40.0f;
	constexpr float UpdateItemY = 0.0f;

	// скорость печати
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

	// ===- Привязка событий Таймлайна -===
	if (LiftingItemCurve)
	{
		FOnTimelineFloat Broker;
		Broker.BindDynamic(this, &AEquippableItem::UpdateTimeline);
		LiftingItemTimeLine->AddInterpFloat(LiftingItemCurve, Broker);

		FOnTimelineEvent FinishedEvent;
		FinishedEvent.BindDynamic(this, &AEquippableItem::OnTimelineFinished);
		LiftingItemTimeLine->SetTimelineFinishedFunc(FinishedEvent);
	}

	// ===- Настройка Динамического Материала -===
	// создаю динамический материал для проброса 2D текстуры виджета в 3D меш дневника
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

	// ===- Инициализация текстовых блоков -===
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
			LeftItemText->SetText(FText::FromString(TEXT(""))); //в начале пусто 
			RightItemText->SetText(FText::FromString(TEXT(""))); //в начале пусто 
		}
	}
}

void AEquippableItem::Equip(USceneComponent* const LabelToAttach)
{
	// прикрепляю предмет к камере игрока + запуск анимации появления
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
	// плавно поднимаю предмет на основе кривой
	const float CurrentHeight_Z = FMath::Lerp(StartHeightZ, FinishHeightZ, Alpha);
	SetActorRelativeLocation(FVector(UpdateItemX, UpdateItemY, CurrentHeight_Z));
}

// ===- Логика текста (печать текста) -===
void AEquippableItem::StartWriting(const FString NewText)
{
	DiaryPages.Add(NewText);
	const int32 IndexNewPage = DiaryPages.Num() - 1;

	//  определить на какую страницу писать (левая,правая)
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

	// подготовка буфера к печати
	FullText = NewText;
	CurrentText = TEXT("");
	if (TargetTextForWriting)
	{
		TargetTextForWriting->SetText(FText::FromString(TEXT("")));
	}

	// запуск таймера печати
	GetWorldTimerManager().SetTimer(NewLetterTimer, this, &AEquippableItem::TypeNextCharacter, TypeSpeedRate, true);
}

void AEquippableItem::TypeNextCharacter()
{
	// по символьно добавляю текст из FullText в CurrentText
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
		// закончился текст
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
	// останавка печати, если игрок перелистнул страницу во время печати текста
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

	// управление перелистыванием
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
	// конца анимации поднятия предмета
	if (LiftingItemTimeLine && LiftingItemTimeLine->GetPlaybackPosition() >= LiftingItemTimeLine->GetTimelineLength())
	{
		bIsEquipped = true;

		ADetectiveCharacter* const Player = Cast<ADetectiveCharacter>(GetOwner());
		if (!Player) return;

		// проверка есть ли новая запись в дневник
		if (Player->BufferLines.Num() > 0)
		{
			const FString NewThought = Player->BufferLines[0];
			StartWriting(NewThought);
			Player->BufferLines.RemoveAt(0); // удаляем мысль из буфера

		}
		else if (LiftingItemTimeLine->GetPlaybackPosition() == 0)
		{
			SetActorHiddenInGame(true);
		}
	}
	// скрытие предмета в конце анимации
	else if (LiftingItemTimeLine->GetPlaybackPosition() <= 0.0f)
	{
		SetActorHiddenInGame(true);
	}
}

