// Fill out your copyright notice in the Description page of Project Settings.


#include "TBS_PlayerController.h"
#include "HumanPlayer.h"

ATBS_PlayerController::ATBS_PlayerController() 
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
}

void ATBS_PlayerController::ClickOnGrid() 
{
	if (AHumanPlayer* HumanPawn = Cast<AHumanPlayer>(GetPawn()))
	{
		HumanPawn->OnClick();
	}
}

void ATBS_PlayerController::BeginPlay() 
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(TBSContext, 0);
	}
}

void ATBS_PlayerController::SetupInputComponent() 
{
	Super::SetupInputComponent();

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Triggered, this, &ATBS_PlayerController::ClickOnGrid);
	}
}