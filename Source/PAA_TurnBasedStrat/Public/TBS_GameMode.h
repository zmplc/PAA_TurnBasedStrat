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

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<AGameField> GameFieldClass;

	// Tipo di AI
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<class ARandomPlayer> AIPlayerClass;

	// Proprietà per fasi partita
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	bool bPlacementPhase = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	int32 HumanTowersControlled = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	int32 AiTowersControlled = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	int32 HumanConsecutiveWithTwoTowers = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	int32 AiConsecutiveWithTwoTowers = 0;

	// Contatore turni totali
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	int32 TurnCounter = 0;

	// Variabile bool per vedere se la partità è finita
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	bool bGameEnded = false;

	// Lo uso per riferimento al widget per poter chiamare la funzione per mostrare overlay vittoria
	UPROPERTY()
	class UUserWidget* MainHUDWidget = nullptr;

	// Classe del widget HUD per poi chiamare la funzione di overlay per vittoria
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	// Numero unità piazzate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Placement")
	int32 UnitsPlaced = 0;

	// Array tile per piazzamento iniziale
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Placement")
	TArray<ATile*> PlacementZoneTiles;

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

	// Funzione per gestire la fine della partita con l'ID del vincitore
	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnGameEnd(int32 WinnerID);

	// Lancio moneta per decidere chi inizia
	UFUNCTION(BlueprintCallable, Category = "Game")
	bool TossCoin();

	// Gestione destroy e respawn quando una unità muore
	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnUnitDied(AUnit* DeadUnit);

	// Funzione per mostrare zona piazzamento
	UFUNCTION(BlueprintCallable, Category = "Game")
	void ShowPlacementZones();

	// Funzione per nascondere zona piazzamento
	UFUNCTION(BlueprintCallable, Category = "Game")
	void HidePlacementZones();

	// Aggiorno contatori delle torri ogni volta che ne viene conquistata una
	UFUNCTION(BlueprintCallable, Category = "Game")
	void UpdateTowerCounts();

	// Funzione per fare il check dello stato delle torri (devo vedere se sono neutrali, sotto controllo o contese)
	UFUNCTION(BlueprintCallable, Category = "Game")
	void CheckTowerStatus();

private:
	FTimerHandle ResetTimerHandle;
};
