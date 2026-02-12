// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerInterface.h"
#include "Unit.h"
#include "Tile.h"
#include "TBS_GameInstance.h"
#include "TBS_PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HumanPlayer.generated.h"

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
	UTBS_GameInstance* GameInstance;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// keeps track of turn
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	bool IsMyTurn = false;

	// Unità selezionata dal player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
	AUnit* SelectedUnit;

	// Classe Sniper da spawnare
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit")
	TSubclassOf<AUnit> SniperClass;

	// Classe Brawler da spawnare
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit")
	TSubclassOf<AUnit> BrawlerClass;

	// Classe da spawnare
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit")
	TSubclassOf<AUnit> ClassToSpawn;

	// Sniper piazzato: y/n
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
	bool bHasPlacedSniper;

	// Brawler piazzato: y/n
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
	bool bHasPlacedBrawler;

	// Tipo di unità da spawnare, se NONE non c'è nessuna unità selezionata
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
	EUnitType PendingUnitTypeToSpawn;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnPlacementTurnStart() override;
	virtual void OnTurnStart() override;
	virtual void OnTurnEnd() override;
	virtual bool PendingTurnActions() const override;
	virtual void SelectUnit(class AUnit* Unit) override;
	virtual void OnWin() override;
	virtual void OnLose() override;

	// called on left mouse click (binding)
	UFUNCTION()
	void OnClick();

	// Funzione per selezione Sniper
	UFUNCTION()
	void SelectSniperForPlacement();

	// Funzione per selezione Brawler
	UFUNCTION()
	void SelectBrawlerForPlacement();
};
