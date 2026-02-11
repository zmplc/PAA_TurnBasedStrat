// Fill out your copyright notice in the Description page of Project Settings.


#include "TBS_GameMode.h"
#include "TBS_PlayerController.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
#include "EngineUtils.h"

ATBS_GameMode::ATBS_GameMode()
{
	DefaultPawnClass = AHumanPlayer::StaticClass();
    PlayerControllerClass = ATBS_PlayerController::StaticClass();

	// Imposto i valori di default
	CurrentPlayer = 0;
	bPlacementPhase = true;
	HumanTowersControlled = 0;
	AITowersControlled = 0;
	HumanConsecutiveWithTwoTowers = 0;
	AIConsecutiveWithTwoTowers = 0;
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

	// Chiamo StartPlacementPhase per vedere se funziona onclick
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay: StartPlacementPhase chiamata"));
	StartPlacementPhase();
	
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

	// Chiamo funzione StartPlacementTurn con il giocatore vincitore di TossCoin
	StartPlacementTurn(CurrentPlayer);
}

void ATBS_GameMode::StartPlacementTurn(int32 PlayerID)
{
	// Chiamo funzione per piazzamento relativo al PlayerID
	if (PlayerID == 0 && HumanPlayer.GetInterface())
	{
		HumanPlayer->OnPlacementTurnStart();
		UE_LOG(LogTemp, Log, TEXT("StartPlacementTurn: turno piazzamento per HumanPlayer"));
	}
	else if (PlayerID == 1 && RandomPlayer.GetInterface())
	{
		RandomPlayer->OnPlacementTurnStart();
		UE_LOG(LogTemp, Log, TEXT("StartPlacementTurn: turno piazzamento per RandomPlayer"));
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
	CurrentPlayer = PlayerID;

	// Chiamo OnTurnStart
	if (PlayerID == 0 && HumanPlayer.GetInterface())
	{
		HumanPlayer->OnTurnStart();
		UE_LOG(LogTemp, Log, TEXT("StartTurn: turno iniziato per HumanPlayer"));
		if (UTBS_GameInstance* GI = GetGameInstance<UTBS_GameInstance>())
		{
			FString Msg = TEXT("Turno di HumanPlayer");
			GI->SetTurnMessage(Msg);
		}
	}
	else if (PlayerID == 1 && RandomPlayer.GetInterface())
	{
		RandomPlayer->OnTurnStart();
		UE_LOG(LogTemp, Log, TEXT("StartTurn: turno iniziato per RandomPlayer"));
		if (UTBS_GameInstance* GI = GetGameInstance<UTBS_GameInstance>())
		{
			FString Msg = TEXT("Turno di AIPlayer");
			GI->SetTurnMessage(Msg);
		}
	}

	// Incremento contatore turni
	TurnCounter++;

}

// Turno successivo con messaggi gameinstance
void ATBS_GameMode::TurnNextPlayer(int32 PlayerID)
{
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
	// Controllo vittoria dopo il cambio turno
	CheckVictoryCondition();
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
		if (HumanConsecutiveWithTwoTowers >= 2)
		{
			// Vittoria
			UE_LOG(LogTemp, Warning, TEXT("Vittoria HumanPlayer: 2 torri controllate per 2 turni consecutivi"));
			if (HumanPlayer.GetInterface()) HumanPlayer->OnWin();
			return;
		}
	}
	else
	{
		// Altrimento resetto contatore di conquiste consecutive
		HumanConsecutiveWithTwoTowers = 0;
	}

	// Check per RandomPlayer
	if (AITowersControlled >= 2)
	{
		AIConsecutiveWithTwoTowers++;
		UE_LOG(LogTemp, Log, TEXT("RandomPlayer controlla %d torri con conquiste consecutive: %d"), AITowersControlled, AIConsecutiveWithTwoTowers);
		if (AIConsecutiveWithTwoTowers >= 2)
		{
			// Vittoria
			UE_LOG(LogTemp, Warning, TEXT("Vittoria RandomPlayer: 2 torri controllate per 2 turni consecutivi"));
			if (RandomPlayer.GetInterface()) RandomPlayer->OnWin();
			return;
		}
	}
	else
	{
		// Altrimento resetto contatore di conquiste consecutive
		AIConsecutiveWithTwoTowers = 0;
	}
}

// Update delle torri controllate dai giocatori
void ATBS_GameMode::UpdateTowerCount(int32 PlayerID, int32 Delta)
{
	// Aggiorno contatore del giocatore corretto, Delta è +1 quando conquista e -1 quando perde torre
	if (PlayerID == 0)
	{
		HumanTowersControlled += Delta;
		UE_LOG(LogTemp, Log, TEXT("UpdateTowerCount: Umano ora controlla %d torri (delta: %d)"), HumanTowersControlled, Delta);
	}
	else if (PlayerID == 1)
	{
		AITowersControlled += Delta;
		UE_LOG(LogTemp, Log, TEXT("UpdateTowerCount: AI (RandomPlayer) ora controlla %d torri (delta: %d)"), AITowersControlled, Delta);
	}

	// Controllo se uno dei player ha vinto
	CheckVictoryCondition();
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
	DeadUnit->RespawnAtInitialPosition(); // TODO: ancora da fare la funzione
}