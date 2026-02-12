// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TBS_PlayerController.generated.h"

/**
 * 
 */

class AHumanPlayer;

UCLASS()
class PAA_TURNBASEDSTRAT_API ATBS_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATBS_PlayerController();

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* TBSContext;


	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ClickAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SelectSniperAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SelectBrawlerAction;

	// Funzione chiamata dal ClickAction
	void ClickOnGrid();
	void SelectSniper();
	void SelectBrawler();
		
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};
