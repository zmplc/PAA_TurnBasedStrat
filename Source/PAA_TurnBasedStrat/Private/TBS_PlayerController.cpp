// Fill out your copyright notice in the Description page of Project Settings.


#include "TBS_PlayerController.h"
#include "HumanPlayer.h"
#include "TBS_GameMode.h"

ATBS_PlayerController::ATBS_PlayerController() 
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
}

void ATBS_PlayerController::ClickOnGrid() 
{
	ATBS_GameMode* GM = GetWorld()->GetAuthGameMode<ATBS_GameMode>();
	AHumanPlayer* HumanPawn = Cast<AHumanPlayer>(GetPawn());
	if (HumanPawn && GM && GM->CurrentPlayer == 0)
	{
		HumanPawn->OnClick();
	}
}

void ATBS_PlayerController::SelectSniper()
{
	ATBS_GameMode* GM = GetWorld()->GetAuthGameMode<ATBS_GameMode>();
	AHumanPlayer* HumanPawn = Cast<AHumanPlayer>(GetPawn());
	if (HumanPawn && GM && GM->CurrentPlayer == 0)
	{
		HumanPawn->SelectSniperForPlacement();
	}
}

void ATBS_PlayerController::SelectBrawler()
{
	ATBS_GameMode* GM = GetWorld()->GetAuthGameMode<ATBS_GameMode>();
	AHumanPlayer* HumanPawn = Cast<AHumanPlayer>(GetPawn());
	if (HumanPawn && GM && GM->CurrentPlayer == 0)
	{
		HumanPawn->SelectBrawlerForPlacement();
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
		EnhancedInputComponent->BindAction(SelectSniperAction, ETriggerEvent::Completed, this, &ATBS_PlayerController::SelectSniper);
		EnhancedInputComponent->BindAction(SelectBrawlerAction, ETriggerEvent::Completed, this, &ATBS_PlayerController::SelectBrawler);
	}
}