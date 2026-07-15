// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TEST_CHARACTER_DANIEL.generated.h"

UCLASS()
class BETA_LIGHTHOUSE_API ATEST_CHARACTER_DANIEL : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATEST_CHARACTER_DANIEL();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
