// Fill out your copyright notice in the Description page of Project Settings.


#include "Gas_Kolonka.h"
#include "Components/BoxComponent.h"
#include "CableComponent.h"
#include "Kismet/GameplayStatics.h"
#include "VWP_PlayerCar.h"
#include "Engine/OverlapResult.h"

AGas_Kolonka::AGas_Kolonka()
{

	PrimaryActorTick.bCanEverTick = false;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Kolonka"));
	RootComponent = BaseMesh;

	StaticMeshHole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticHole"));
	StaticMeshHole->SetupAttachment(BaseMesh);
	
	Pistolet = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pistolet"));
	Pistolet->SetupAttachment(BaseMesh);
	Pistolet->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxTrigger"));
	InteractTrigger->SetupAttachment(Pistolet);
	InteractTrigger->SetBoxExtent(FVector(20.0f, 20.0f, 20.0f));
	InteractTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractTrigger->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	CableMesh = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
	CableMesh->SetupAttachment(BaseMesh);
	CableMesh->EndLocation = FVector::ZeroVector;
	CableMesh->SetVisibility(false);
	CableMesh->SetComponentTickEnabled(false);

	IdleNozzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("IdleNozzleLocation"));
	IdleNozzleLocation->SetupAttachment(BaseMesh);

}




void AGas_Kolonka::TogglePistolet()
{
	if (bIsAttecheToCar)
	{
		Pistolet->AttachToComponent(IdleNozzleLocation, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		bIsAttecheToCar = false;

		CableMesh->SetVisibility(false);
		CableMesh->SetComponentTickEnabled(false);
		StaticMeshHole->SetVisibility(true);

		if(CurrentRefuelingCar)
		{
			CurrentRefuelingCar->SetRefuelingState(false);
			CurrentRefuelingCar = nullptr;
		}
		return;
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(MaxCableLeght);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_Pawn, Sphere, QueryParams);
	AVWP_PlayerCar* NearestCar = nullptr;
	float ClosesDistance = MaxCableLeght;

	for (const FOverlapResult& Result : OverlapResults)
	{
		if (AVWP_PlayerCar* FoundCar = Cast<AVWP_PlayerCar>(Result.GetActor()))
		{
			float Dist = FVector::Dist(GetActorLocation(), FoundCar->GetActorLocation());
			if (Dist < ClosesDistance)
			{
				ClosesDistance = Dist;
				NearestCar = FoundCar;
			}
		}
	}

	if (!NearestCar)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("RADOM NET CAR!"));
		return;
	}

	if (!NearestCar->IsGasCoverOpen())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("PLEASE, OPEN BAK"));
		return;
	}

	FVector CarCenter = NearestCar->GetActorLocation();
	FVector DirToPump = (GetActorLocation() - CarCenter).GetSafeNormal();
	FVector DirToHole = (NearestCar->GetGasHoleLocation()->GetComponentLocation() - CarCenter).GetSafeNormal();

	if (FVector::DotProduct(DirToPump, DirToHole) < 0.0f)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("TI VSTAL NE TOY STORONOY"));
		return;
	}

	Pistolet->AttachToComponent(NearestCar->GetGasHoleLocation(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	bIsAttecheToCar = true;

	StaticMeshHole->SetVisibility(false);
	CableMesh->SetVisibility(true);
	CableMesh->SetComponentTickEnabled(true);


	CurrentRefuelingCar = NearestCar;
	CurrentRefuelingCar->SetRefuelingState(true);
}


void AGas_Kolonka::BeginPlay()
{
	Super::BeginPlay();

	if (Pistolet && CableMesh)
	{
		CableMesh->SetAttachEndToComponent(Pistolet);
	}
}

void AGas_Kolonka::Interact(AActor* Interactor, UPrimitiveComponent* HitComponent)
{
	if (HitComponent == InteractTrigger)
	{
		TogglePistolet();
	}
}

void AGas_Kolonka::OnFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) {}
void AGas_Kolonka::OnLostFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) {}
void AGas_Kolonka::Inspect(AActor* Interactor, UPrimitiveComponent* HitComponent) {}
