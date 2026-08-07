// Fill out your copyright notice in the Description page of Project Settings.


#include "FuelPump.h"
#include "Components/BoxComponent.h"
#include "CableComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerVehicle.h"
#include "Engine/OverlapResult.h"

AFuelPump::AFuelPump()
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
	InteractTrigger->SetCollisionResponseToAllChannels(ECR_Overlap);
	InteractTrigger->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	CableMesh = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
	CableMesh->SetupAttachment(BaseMesh);
	CableMesh->EndLocation = FVector::ZeroVector;
	CableMesh->SetVisibility(false);
	CableMesh->SetComponentTickEnabled(false);

	IdleNozzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("IdleNozzleLocation"));
	IdleNozzleLocation->SetupAttachment(BaseMesh);

	MaxCableLeght = 500.0f;
	bIsAttecheToCar = false;
	CurrentRefuelingCar = nullptr;
}

void AFuelPump::TogglePistolet()
{
	// returning the nozzle to the dispenser
	if (bIsAttecheToCar)
	{
		Pistolet->AttachToComponent(IdleNozzleLocation, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		bIsAttecheToCar = false;

		if (CurrentRefuelingCar)
		{
			CurrentRefuelingCar->ConnectedPump = nullptr;
			
			CurrentRefuelingCar->SetRefuelingState(false);
			CurrentRefuelingCar = nullptr;
			
		}

		CableMesh->SetVisibility(false);
		CableMesh->SetComponentTickEnabled(false);
		StaticMeshHole->SetVisibility(true);
		return;
	}

	// car search
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(MaxCableLeght);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// search for all actors within a radius
	GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_Pawn, Sphere, QueryParams);
	
	APlayerVehicle* NearestCar = nullptr;
	float ClosesDistance = MaxCableLeght;

	// searching for the nearest vehicle
	for (const FOverlapResult& Result : OverlapResults)
	{
		if (APlayerVehicle* FoundCar = Cast<APlayerVehicle>(Result.GetActor()))
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

	// Checking the correct side of the vehicle
	FVector CarCenter = NearestCar->GetActorLocation();
	FVector DirToPump = (GetActorLocation() - CarCenter).GetSafeNormal();
	FVector DirToHole = (NearestCar->GetGasHoleLocation()->GetComponentLocation() - CarCenter).GetSafeNormal();

	if (FVector::DotProduct(DirToPump, DirToHole) < 0.0f)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("TI VSTAL NE TOY STORONOY"));
		return;
	}

	// connection 
	Pistolet->AttachToComponent(NearestCar->GetGasHoleLocation(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	bIsAttecheToCar = true;

	NearestCar->ConnectedPump = this;

	StaticMeshHole->SetVisibility(false);
	CableMesh->SetVisibility(true);
	CableMesh->SetComponentTickEnabled(true);

	CurrentRefuelingCar = NearestCar;
	CurrentRefuelingCar->SetRefuelingState(true);
}

void AFuelPump::BeginPlay()
{
	Super::BeginPlay();

	if (Pistolet && CableMesh)
	{
		CableMesh->SetAttachEndToComponent(Pistolet);
	}
}

void AFuelPump::Interact(AActor* Interactor, UPrimitiveComponent* HitComponent)
{
	if (HitComponent && HitComponent->GetOwner() == this)
	{
		TogglePistolet();
	}
}

void AFuelPump::OnFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) {}
void AFuelPump::OnLostFocus(AActor* Interactor, UPrimitiveComponent* HitComponent) {}
void AFuelPump::Inspect(AActor* Interactor, UPrimitiveComponent* HitComponent) {}
