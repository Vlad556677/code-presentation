// Fill out your copyright notice in the Description page of Project Settings.


#include "TEST_CHARACTER_DANIEL.h"

// Sets default values
ATEST_CHARACTER_DANIEL::ATEST_CHARACTER_DANIEL()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATEST_CHARACTER_DANIEL::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATEST_CHARACTER_DANIEL::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATEST_CHARACTER_DANIEL::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

