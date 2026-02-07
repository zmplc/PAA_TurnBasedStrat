// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PlayerInterface.h"
#include "TBS_GameInstance.h"
#include "GameField.h"
#include "Unit.h"
#include "ConfigData.h"
#include "TBS_GameMode.generated.h"

class AHumanPlayer;
class AAIPlayer;

/**
 * 
 */
UCLASS()
class PAA_TURNBASEDSTRAT_API ATBS_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATBS_GameMode();

	virtual void BeginPlay() override;

	// Riferimenti giocatori tramite interfaccia
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Players")
	TScriptInterface<IPlayerInterface> HumanPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Players")
	TScriptInterface<IPlayerInterface> RandomPlayer;

	// Giocatore corrente (0 = umano, 1 = AI)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	int32 CurrentPlayer = 0;

	// Variabile per memorizzare il player iniziale
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	int32 FirstPlayer = 0;

	// field size
	int32 FieldSize;

	// tile padding percentage
	float CellPadding;

	// tile size
	float TileSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	UConfigData* GridData;

	// Griglia di gioco
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Field")
	AGameField* GField;

	// Classi configurabili dall'editor
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<AGameField> GameFieldClass;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<AUnit> SniperClass;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<AUnit> BrawlerClass;

	// Proprietà per fasi partita
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	bool bPlacementPhase = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	int32 HumanTowersControlled = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	int32 AITowersControlled = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	int32 HumanConsecutiveWithTwoTowers = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	int32 AIConsecutiveWithTwoTowers = 0;

	// Contatore turni totali
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	int32 TurnCounter = 0;

	// Numero unità piazzate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Placement")
	int32 UnitsPlaced = 0;

	// METODI
	// Inizio fase piazzamento unità
	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartPlacementPhase();

	// Inizio turno piazzamento di un giocatore
	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartPlacementTurn(int32 PlayerID);

	// Chiamata quando un giocatore ha piazzato un'unità
	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnUnitPlaced(int32 PlayerID);

	// Inizio turno di un giocatore
	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartTurn(int32 PlayerID);

	// Fine turno e passo al prossimo dell'altro giocatore
	UFUNCTION(BlueprintCallable, Category = "Game")
	void TurnNextPlayer(int32 PlayerID);

	// Controllo condizione vittoria (un giocatore controlla 2 torri su 3 contemporaneamente per 2 turni consecutivi)
	UFUNCTION(BlueprintCallable, Category = "Game")
	void CheckVictoryCondition();

	// Lancio moneta per decidere chi inizia
	UFUNCTION(BlueprintCallable, Category = "Game")
	bool TossCoin();

	// Aggiorno contatori delle torri ogni volta che ne viene conquistata una (contatore legato al PlayerID)
	UFUNCTION(BlueprintCallable, Category = "Game")
	void UpdateTowerCount(int32 PlayerID, int32 Delta);

	// Gestione destroy e respawn quando una unità muore
	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnUnitDied(AUnit* DeadUnit);

private:
	FTimerHandle ResetTimerHandle;
};
