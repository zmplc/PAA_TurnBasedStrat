// Fill out your copyright notice in the Description page of Project Settings.


#include "TBS_GameMode.h"
#include "TBS_PlayerController.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"

ATBS_GameMode::ATBS_GameMode()
{
	DefaultPawnClass = AHumanPlayer::StaticClass();
    PlayerControllerClass = ATBS_PlayerController::StaticClass();

	// Imposto i valori di default
	CurrentPlayer = 0;
	bPlacementPhase = true;
	HumanTowersControlled = 0;
	AiTowersControlled = 0;
	HumanConsecutiveWithTwoTowers = 0;
	AiConsecutiveWithTwoTowers = 0;
	TurnCounter = 0;

	UE_LOG(LogTemp, Log, TEXT("GameMode creato e inizializzato"));
}

void ATBS_GameMode::BeginPlay()
{
	Super::BeginPlay();

	AHumanPlayer* HumanPawn = GetWorld()->GetFirstPlayerController()->GetPawn<AHumanPlayer>();

	if (!IsValid(HumanPawn))
	{
		UE_LOG(LogTemp, Error, TEXT("No player pawn of type '%s' was found"), *AHumanPlayer::StaticClass()->GetName());
		return;
	}

	// Inizializzo PlayerInterface per HumanPlayer
	if (HumanPawn->Implements<UPlayerInterface>())
	{
		HumanPlayer.SetObject(HumanPawn);
		HumanPlayer.SetInterface(Cast<IPlayerInterface>(HumanPawn));
		UE_LOG(LogTemp, Log, TEXT("HumanPlayer interface inizializzata"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HumanPlayer interface non inizializzata"));
		return;
	}

	if (AIPlayerClass)
	{
		FVector AISpawnLocation = FVector(0, 0, -1000);
		ARandomPlayer* AIPlayer = GetWorld()->SpawnActor<ARandomPlayer>(AIPlayerClass, AISpawnLocation, FRotator::ZeroRotator);

		if (AIPlayer && AIPlayer->Implements<UPlayerInterface>())
		{
			RandomPlayer.SetObject(AIPlayer);
			RandomPlayer.SetInterface(Cast<IPlayerInterface>(AIPlayer));
			UE_LOG(LogTemp, Log, TEXT("RandomPlayer spawnato e inizializzato correttamente"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Errore nello spawn di RandomPlayer o interfaccia mancante"));
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AIPlayerClass non assegnato (devo controllare BP)"));
		return;
	}

	if (GridData)
	{
		FieldSize = GridData->GridSize;
		TileSize = GridData->TileSize;
		CellPadding = GridData->CellPadding;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GridData has not been assigned."));
		return;
	}

	if (GameFieldClass != nullptr)
	{
		GField = GetWorld()->SpawnActor<AGameField>(GameFieldClass);
		UE_LOG(LogTemp, Log, TEXT("Game Field spawnato"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Game Field is null"));
		return;
	}

	if (HUDWidgetClass)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			MainHUDWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
			if (MainHUDWidget)
			{
				MainHUDWidget->AddToViewport();
				UE_LOG(LogTemp, Log, TEXT("GameMode: HUD Widget creato e mostrato"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode: HUDWidgetClass non assegnato!"));
	}

	// Reset HUD
	UTBS_GameInstance* GI = GetGameInstance<UTBS_GameInstance>();
	if (GI)
	{
		GI->ResetGame();
	}
	
	StartPlacementPhase();

	// Siccome per il MainMenu ho messo input mode UI only, quando inizia la partita devo abilitare di nuovo l'input a modalità normale
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
	}
}

// Funzione lancio moneta
bool ATBS_GameMode::TossCoin()
{
	// Random: true = human vince, false = AI vince
	bool bHumanWins = FMath::RandBool();

	UE_LOG(LogTemp, Log, TEXT("Lancio moneta: %s vince!"), bHumanWins ? TEXT("Human") : TEXT("AI"));

	return bHumanWins;
}

// Fase piazzamento iniziale
void ATBS_GameMode::StartPlacementPhase()
{
	// Attivo fase piazzamento
	bPlacementPhase = true;

	// Resetto i contatori
	UnitsPlaced = 0;
	TurnCounter = 0;

	// Chiamo la funzione TossCoin per vedere chi inizia
	bool bHumanStarts = TossCoin();
	// Imposto chi è il CurrentPlayer
	CurrentPlayer = bHumanStarts ? 0 : 1;
	FirstPlayer = CurrentPlayer;

	// Log
	UE_LOG(LogTemp, Log, TEXT("Fase di piazzamento unità iniziata"));

	UTBS_GameInstance* GI = GetGameInstance<UTBS_GameInstance>();
	if (!GI) return;

	GI->SetTurnOwner(TEXT("Lancio moneta..."));
	GI->SetTurnMessage(TEXT("Lancio moneta per decidere chi inizia..."));

	// Dopo 1 secondo mostro lancio in corso
	FTimerHandle Timer1;
	GetWorld()->GetTimerManager().SetTimer(Timer1, [this, GI]()
		{
			GI->SetTurnMessage(TEXT("Lancio in corso..."));
		}, 2.0f, false);

	// Dopo mostro chi ha vinto il lancio
	FTimerHandle Timer2;
	FString WinnerText = bHumanStarts ? TEXT("HumanPlayer") : TEXT("AI");
	GetWorld()->GetTimerManager().SetTimer(Timer2, [this, GI, WinnerText]()
		{
			GI->SetTurnMessage(FString::Printf(TEXT("%s vince e inizia!"), *WinnerText));
		}, 4.0f, false);
	// Chiamo funzione ShowPlacementZones per mostrare le zone di piazzamento per entrambi i player
	ShowPlacementZones();

	// Dopo faccio iniziare fase di piazzamento per CurrentPlayer
	FTimerHandle Timer3;
	GetWorld()->GetTimerManager().SetTimer(Timer3, [this]()
		{
			// Chiamo funzione StartPlacementTurn con il giocatore vincitore di TossCoin
			StartPlacementTurn(CurrentPlayer);
		}, 6.0f, false);
}

void ATBS_GameMode::StartPlacementTurn(int32 PlayerID)
{
	// Chiamo funzione per piazzamento relativo al PlayerID
	if (PlayerID == 0 && HumanPlayer.GetInterface())
	{
		HumanPlayer->OnPlacementTurnStart();
		UE_LOG(LogTemp, Log, TEXT("StartPlacementTurn: turno piazzamento per HumanPlayer"));
		if (UTBS_GameInstance* GI = GetGameInstance<UTBS_GameInstance>())
		{
			GI->SetTurnOwner(TEXT("Turno: Giocatore"));
		}
	}
	else if (PlayerID == 1 && RandomPlayer.GetInterface())
	{
		RandomPlayer->OnPlacementTurnStart();
		UE_LOG(LogTemp, Log, TEXT("StartPlacementTurn: turno piazzamento per RandomPlayer"));
		if (UTBS_GameInstance* GI = GetGameInstance<UTBS_GameInstance>())
		{
			GI->SetTurnOwner(TEXT("Turno: AI"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("StartPlacementTurn: giocatore %d non valido o interfaccia mancante"), CurrentPlayer);
	}
}

// In base alle unità piazzate gestisco la fase di piazzamento iniziale
// una volta che sono state spawnate 4 unità totali faccio partire il turno normale con il player che ha vinto il lancio della moneta
void ATBS_GameMode::OnUnitPlaced(int32 PlayerID)
{
	// Incremento contatore totale unità piazzate
	UnitsPlaced++;
	UE_LOG(LogTemp, Log, TEXT("Unità piazzata"));

	// Se tutte le unità sono state piazzate (totale 4, 2 per ciascun player)
	if (UnitsPlaced >= 4)
	{
		UE_LOG(LogTemp, Log, TEXT("Piazzamento delle unità completato"));
		// Nascondo zone piazzamento iniziale
		HidePlacementZones();
		// Fine fase di piazzamento
		bPlacementPhase = false;
		// Inizio partita
		StartTurn(FirstPlayer);
		return;
	}
	// Altrimento passo al prossimo giocatore per piazzare le unità
	CurrentPlayer = (PlayerID + 1) % 2;
	StartPlacementTurn(CurrentPlayer);
}

// Inizio turno con messaggi gameinstance
void ATBS_GameMode::StartTurn(int32 PlayerID)
{
	// Se la partità è terminata devo bloccare il turno
	if (bGameEnded) return;

	CurrentPlayer = PlayerID;

	// Per ogni turno devo: fare un check dello status delle torri, aggiornare i contatori delle torri controllare da HumanPlayer e dall'AI e controllare se qualcuno ha vinto
	CheckTowerStatus();
	UpdateTowerCounts();
	CheckVictoryCondition();

	// Chiamo OnTurnStart
	if (PlayerID == 0 && HumanPlayer.GetInterface())
	{
		HumanPlayer->OnTurnStart();
		UE_LOG(LogTemp, Log, TEXT("StartTurn: turno iniziato per HumanPlayer"));
		if (UTBS_GameInstance* GI = GetGameInstance<UTBS_GameInstance>())
		{
			GI->SetTurnOwner(TEXT("Turno: Giocatore"));
		}
	}
	else if (PlayerID == 1 && RandomPlayer.GetInterface())
	{
		RandomPlayer->OnTurnStart();
		UE_LOG(LogTemp, Log, TEXT("StartTurn: turno iniziato per RandomPlayer"));
		if (UTBS_GameInstance* GI = GetGameInstance<UTBS_GameInstance>())
		{
			GI->SetTurnOwner(TEXT("Turno: AI"));
		}
	}

	// Incremento contatore turni
	TurnCounter++;
}

// Turno successivo con messaggi gameinstance
void ATBS_GameMode::TurnNextPlayer(int32 PlayerID)
{
	// Se la partità è terminata devo bloccare il turno
	if (bGameEnded) return;

	UE_LOG(LogTemp, Log, TEXT("TurnNextPlayer: Fine turno di %d"), CurrentPlayer);

	// Chiamo OnTurnEnd
	if (PlayerID == 0 && HumanPlayer.GetInterface())
	{
		HumanPlayer->OnTurnEnd();
		UE_LOG(LogTemp, Log, TEXT("EndTurn: fine turno per HumanPlayer"));
	}
	else if (PlayerID == 1 && RandomPlayer.GetInterface())
	{
		RandomPlayer->OnTurnEnd();
		UE_LOG(LogTemp, Log, TEXT("EndTurn: fine turno per RandomPlayer"));
	}

	// Passo al giocatore successivo
	CurrentPlayer = (CurrentPlayer + 1) % 2;
	// Incremento contatore turni
	TurnCounter++;
	UE_LOG(LogTemp, Log, TEXT("TurnNextPlayer: Inizia turno n° %d per %d"), TurnCounter, CurrentPlayer);

	// Avvio turno per CurrentPlayer
	StartTurn(CurrentPlayer);
}

// Controllo se uno dei due player ha vinto
void ATBS_GameMode::CheckVictoryCondition()
{
	// NB: devo controllare i turni con 2 TORRI CONQUISTATE DALLO STESSO PLAYER
	// Log
	UE_LOG(LogTemp, Log, TEXT("CheckVictoryCondition: controllo vittoria per il turno n° %d"), TurnCounter);

	// Check per HumanPlayer
	if (HumanTowersControlled >= 2)
	{
		HumanConsecutiveWithTwoTowers++;
		UE_LOG(LogTemp, Log, TEXT("HumanPlayer controlla %d torri con conquiste consecutive: %d"), HumanTowersControlled, HumanConsecutiveWithTwoTowers);
		if (HumanConsecutiveWithTwoTowers >= 3)
		{
			// HumanPlayer vince
			OnGameEnd(0);
			return;
		}
	}
	else
	{
		// Altrimento resetto contatore di conquiste consecutive
		HumanConsecutiveWithTwoTowers = 0;
	}

	// Check per RandomPlayer
	if (AiTowersControlled >= 2)
	{
		AiConsecutiveWithTwoTowers++;
		if (AiConsecutiveWithTwoTowers >= 3)
		{
			// AI vince
			OnGameEnd(1);
			return;
		}
	}
	else
	{
		// Altrimento resetto contatore di conquiste consecutive
		AiConsecutiveWithTwoTowers = 0;
	}
}

void ATBS_GameMode::OnGameEnd(int32 WinnerID)
{
	// Se la partità è già finita faccio return
	if (bGameEnded) return;

	UE_LOG(LogTemp, Warning, TEXT("GameMode: Partita terminata - Vincitore: %d"), WinnerID);

	// Imposto la variabile di fine partita a true
	bGameEnded = true;

	// Subito dopo che la partita finisce devo fermare tutti i timer dell'AI altrimenti continua a fare le sue azioni
	if (RandomPlayer.GetInterface())
	{
		if (ARandomPlayer* RP = Cast<ARandomPlayer>(RandomPlayer.GetObject()))
		{
			GetWorld()->GetTimerManager().ClearAllTimersForObject(RP);
		}
	}

	// Chiamo OnWin o OnLose in base al WinnerID e per humanplayer e AI
	if (WinnerID == 0)
	{
		if (HumanPlayer.GetInterface())
		{
			HumanPlayer->OnWin();
		}
		if (RandomPlayer.GetInterface())
		{
			RandomPlayer->OnLose();
		}
	}
	else if (WinnerID == 1)
	{
		if (HumanPlayer.GetInterface())
		{
			HumanPlayer->OnLose();
		}
		if (RandomPlayer.GetInterface())
		{
			RandomPlayer->OnWin();
		}
	}

	if (MainHUDWidget)
	{
		UFunction* ShowVictoryFunction = MainHUDWidget->FindFunction(FName("ShowVictoryOverlay"));
		if (ShowVictoryFunction)
		{
			// Devo passare WinnerID come parametro a ShowVictoryOverlay, uso uno struct
			struct FShowVictoryParams
			{
				int32 WinnerID;
			};

			FShowVictoryParams Params;
			Params.WinnerID = WinnerID;

			MainHUDWidget->ProcessEvent(ShowVictoryFunction, &Params);
		}
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
	}

}

void ATBS_GameMode::UpdateTowerCounts()
{
	if (!GField) return;

	// Getter per ottenere tutte le torri
	TArray<ATower*> Towers = GField->GetTowers();

	// Inizializzazione contatori
	int32 NewHumanTowers = 0;
	int32 NewAiTowers = 0;

	// Per ogni torre controllo se ha stato CONTROLLED, se lo è aggirono i contatori in base al player che la controlla
	for (ATower* Tower : Towers)
	{
		if (Tower && Tower->GetCurrentStatus() == ETowerStatus::CONTROLLED)
		{
			if (Tower->GetControllingPlayerID() == 0)
			{
				NewHumanTowers++;
			}
			else if (Tower->GetControllingPlayerID() == 1)
			{
				NewAiTowers++;
			}
		}
	}

	// Aggiorno le variabili che uso in CheckVictoryCondition
	HumanTowersControlled = NewHumanTowers;
	AiTowersControlled = NewAiTowers;

	UE_LOG(LogTemp, Log, TEXT("GameMode: Torri aggiornate - Human: %d, AI: %d"), HumanTowersControlled, AiTowersControlled);
}

void ATBS_GameMode::CheckTowerStatus()
{
	if (!GField) return;

	// Getter per ottenere tutte le torri
	TArray<ATower*> Towers = GField->GetTowers();
	
	// Per ogni torre chiamo CheckCaptureZone per verificare la zona di cattura della torre
	for (ATower* Tower : Towers)
	{
		if (Tower)
		{
			Tower->CheckCaptureZone(GField);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GameMode: CheckTowerStatus completato"));
}

// Se una unità muore devo gestire il respawn della stessa
void ATBS_GameMode::OnUnitDied(AUnit* DeadUnit)
{
	if (!DeadUnit)
	{
		UE_LOG(LogTemp, Error, TEXT("OnUnitDied: unità nulla"));
		return;
	}

	// Respawn alla posizione iniziale
	DeadUnit->RespawnAtInitialPosition();
}

void ATBS_GameMode::ShowPlacementZones()
{
	PlacementZoneTiles.Empty();

	// Y per HumanPlayer
	for (int32 Y = 0; Y <= 2; ++Y)
	{
		for (int32 X = 0; X < GField->GridSizeX; ++X)
		{
			ATile* Tile = GField->GetTileAtPosition(X, Y);
			if (Tile && Tile->IsWalkable())
			{
				Tile->ShowPlacementOverlay(true);
				PlacementZoneTiles.Add(Tile);
			}
		}
	}

	// Y per AIPlayer
	for (int32 Y = 22; Y <= 24; ++Y)
	{
		for (int32 X = 0; X < GField->GridSizeX; ++X)
		{
			ATile* Tile = GField->GetTileAtPosition(X, Y);
			if (Tile && Tile->IsWalkable())
			{
				Tile->ShowPlacementOverlay(true);
				PlacementZoneTiles.Add(Tile);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GameMode: %d tile di piazzamento evidenziate"), PlacementZoneTiles.Num());
}

void ATBS_GameMode::HidePlacementZones()
{
	for (ATile* Tile : PlacementZoneTiles)
	{
		if (Tile && IsValid(Tile))
		{
			Tile->ShowPlacementOverlay(false);
		}
	}

	PlacementZoneTiles.Empty();
	UE_LOG(LogTemp, Log, TEXT("GameMode: Zone piazzamento nascoste"));
}
