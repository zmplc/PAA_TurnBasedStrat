// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerInterface.h"
#include "TBS_GameInstance.h"
#include "TBS_PlayerController.h"
#include "TBS_GameMode.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HumanPlayer.generated.h"

class AUnit;
class ATile;
class ATBS_GameMode;
class UTBS_GameInstance;

UCLASS()
class PAA_TURNBASEDSTRAT_API AHumanPlayer : public APawn, public IPlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AHumanPlayer();

	// camera component attacched to player pawn
	UCameraComponent* Camera;

	// game instance reference
	UGameInstance* GameInstance;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// keeps track of turn
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	bool IsMyTurn = false;

	// Unità selezionata dal player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
	AUnit* SelectedUnit;

	// Unità da spawnare
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Placement")
	TSubclassOf<AUnit> UnitToSpawnClass;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnPlacementTurnStart() override;
	virtual void OnTurnStart() override;
	virtual void OnTurnEnd() override;
	virtual void PerformTurnActions() override;
	virtual bool PendingTurnActions() const override;
	virtual void SelectUnit(class AUnit* Unit) override;
	virtual void OnWin() override;
	virtual void OnLose() override;

	// called on left mouse click (binding)
	UFUNCTION()
	void OnClick();
};
