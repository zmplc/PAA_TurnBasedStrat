// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomPlayer.h"
#include "GameField.h"
#include "TBS_GameMode.h"
#include "Unit.h"
#include "Tile.h"
#include "Tower.h"
#include "EngineUtils.h"

// Sets default values
ARandomPlayer::ARandomPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GameInstance = Cast<UTBS_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	// Inizializzo valori default
	PlayerID = 1;
	ClassToSpawn = nullptr;
	IsMyTurn = false;
	bHasPlacedSniper = false;
	bHasPlacedBrawler = false;
	CurrentUnitTile = nullptr;
}

// Called when the game starts or when spawned
void ARandomPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARandomPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// ANNULLA IL TIMER!
	// Questo impedisce che la funzione venga eseguita dopo che il mondo è stato distrutto.
	GetWorld()->GetTimerManager().ClearTimer(AI_TurnTimerHandle);
}

// Called every frame
void ARandomPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARandomPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ARandomPlayer::OnPlacementTurnStart()
{
	UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Turno di piazzamento iniziato"));
	IsMyTurn = true;

	// Messaggio che AI sta piazzando
	if (GameInstance)
	{
		GameInstance->SetTurnMessage(TEXT("L'AI sta piazzando..."));
	}

	// Piazzamento automatico dopo il timer di 1.5f
	GetWorld()->GetTimerManager().SetTimer(AI_TurnTimerHandle, this, &ARandomPlayer::PlaceUnitAutomatically, 1.5f, false);
}

void ARandomPlayer::OnTurnStart()
{
	IsMyTurn = true;
	UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Turno AI"));
	// Reset delle unità e del turno
	for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
	{
		AUnit* Unit = *It;
		if (Unit && Unit->OwnerPlayerID == PlayerID && Unit->IsAlive())
		{
			Unit->ResetTurnStatus();
		}
	}
	GameInstance->SetTurnMessage(TEXT("L'AI sta pensando alle sue prossime mosse..."));

	// Dopo timer di 1,5s faccio partire le mosse dell'AI
	GetWorld()->GetTimerManager().SetTimer(AI_TurnTimerHandle, this, &ARandomPlayer::PerformTurnActions, 1.5f, false);
}

void ARandomPlayer::OnTurnEnd()
{
	IsMyTurn = false;
	UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Turno terminato"));
}

void ARandomPlayer::PerformTurnActions()
{
	ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GField || GM->bGameEnded) return;

	UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Inizio esecuzione azioni dell'AI"));

	// Trovo tutte le unità vive dell'AI
	TArray<AUnit*> AiUnits;
	for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
	{
		AUnit* Unit = *It;
		if (Unit && Unit->OwnerPlayerID == PlayerID && Unit->IsAlive())
		{
			AiUnits.Add(Unit);
		}
	}
	// Controllo se ci sono le unità
	if (AiUnits.Num() == 0)
	{
		if (GameInstance)
		{
			GameInstance->SetTurnMessage(TEXT("AI: Nessuna unita' viva"));
		}
		GM->TurnNextPlayer(PlayerID);
		return;
	}

	// Ora devo dividere le possibili azioni: chiamo DecideTarget per decidere se andare verso torre o nemico
	// Poi chiamo NextMoveTowardsTarget per calcolare la prossima mossa usando A*
	// Poi muovo l'unità
	// Poi attacco nemico con meno hp se in range
	// Infine termino il turno e passo a HumanPlayer sempre usando un timer

	// Uso la funzione ProcessUnit perché voglio mostrare (da specifiche) range di movimento delle unità dell'AI quindi ho bisogno di usare dei timer
	ProcessUnit(AiUnits, 0, GM);
}

void ARandomPlayer::OnWin()
{
	GameInstance->SetTurnMessage(TEXT("L'AI ha vinto la partita!"));
	
	// Nel caso di fine partita per RandomPlayer devo annullare tutti i timer attivi e disattivare il range movimento e icone di attacco come fatto per HumanPlayer
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	HideMovementRange(GetWorld()->GetAuthGameMode<ATBS_GameMode>()->GField);
	HideAttackIndicators(GetWorld()->GetAuthGameMode<ATBS_GameMode>()->GField);
}

void ARandomPlayer::OnLose()
{
	GameInstance->SetTurnMessage(TEXT("L'AI ha perso la partita!"));
	
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	HideMovementRange(GetWorld()->GetAuthGameMode<ATBS_GameMode>()->GField);
	HideAttackIndicators(GetWorld()->GetAuthGameMode<ATBS_GameMode>()->GField);
}

void ARandomPlayer::PlaceUnitAutomatically()
{
	ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode());
	if (!GM)
	{
		UE_LOG(LogTemp, Error, TEXT("RandomPlayer: GameMode non trovato!"));
		return;
	}

	if (!GM->bPlacementPhase)
	{
		UE_LOG(LogTemp, Error, TEXT("RandomPlayer: Non è più la fase di piazzamento!"));
		return;
	}

	if (!IsMyTurn)
	{
		UE_LOG(LogTemp, Error, TEXT("RandomPlayer: Non è il mio turno!"));
		return;
	}

	// Devo decidere quale unità spawnare per prima
	FString UnitName;
	bool bIsSniper = false;

	if (!bHasPlacedSniper && !bHasPlacedBrawler)
	{
		// Scelgo casualmente con RandBool se è sniper il primo da spawnare
		bIsSniper = FMath::RandBool();

		if (bIsSniper)
		{
			// Se lo è spawno
			ClassToSpawn = SniperClass;
			UnitName = TEXT("Sniper");
		}
		else
		{
			// Altrimenti spawno brawler
			ClassToSpawn = BrawlerClass;
			UnitName = TEXT("Brawler");
		}

		UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Scelta unità dell'AI: %s"), *UnitName);
	}
	// Una volta piazzata la prima unità devo spawnare la seconda
	else if (!bHasPlacedSniper)
	{
		// Caso sniper
		ClassToSpawn = SniperClass;
		UnitName = TEXT("Sniper");
		bIsSniper = true;
		UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Secondo turno, Sniper piazzato"));
	}
	else if (!bHasPlacedBrawler)
	{
		// Caso brawler
		ClassToSpawn = BrawlerClass;
		UnitName = TEXT("Brawler");
		bIsSniper = false;
		UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Secondo turno, Brawler piazzato"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AI: Tutte le unità già piazzate!"));
		return;
	}

	// Verifico che la classe da spawnare sia assegnata correttamente
	if (!ClassToSpawn)
	{
		UE_LOG(LogTemp, Error, TEXT("RandomPlayer: ClassToSpawn è null"));
		GameInstance->SetTurnMessage(TEXT("Errrore: Classe unità non assegnata"));
		return;
	}

	// Ora devo trovare una tile valida casuale in Y=22,23,24
	ATile* TargetTile = FindRandomValidTile(GM->GField);

	if (!TargetTile)
	{
		UE_LOG(LogTemp, Error, TEXT("RandomPlayer: Nessuna tile valida trovata"));
		GameInstance->SetTurnMessage(TEXT("Errore: Nessuna tile valida trovata"));
		return;
	}

	// Spawno l'unità nella tile
	FVector SpawnLoc = TargetTile->GetActorLocation();
	SpawnLoc.Z += 150.f;
	FRotator SpawnRotation = FRotator(0.f, -90.f, 0.f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


	AUnit* NewUnit = GetWorld()->SpawnActor<AUnit>(ClassToSpawn, SpawnLoc, SpawnRotation, SpawnParams);

	if (NewUnit)
	{
		FVector2D TilePos = TargetTile->GetGridPosition();
		NewUnit->OwnerPlayerID = 1;
		NewUnit->SetCurrentGridPosition(TilePos);
		NewUnit->InitialGridPosition = TilePos;

		NewUnit->SetActorHiddenInGame(false);
		NewUnit->SetActorEnableCollision(true);

		// Segno l'unità come piazzata
		if (bIsSniper)
		{
			bHasPlacedSniper = true;
			UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Sniper piazzato con successo"));
		}
		else
		{
			bHasPlacedBrawler = true;
			UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Brawler piazzato con successo"));
		}

		int32 X = FMath::RoundToInt(TilePos.X);
		int32 Y = FMath::RoundToInt(TilePos.Y);
		UE_LOG(LogTemp, Log, TEXT("RandomPlayer: %s piazzato in (%d, %d)"), *UnitName, X, Y);
		GameInstance->SetTurnMessage(FString::Printf(TEXT("L'AI ha piazzato %s"), *UnitName));

		// Notifico GameMode che l'unità è stata piazzata
		GM->OnUnitPlaced(1);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RandomPlayer: Spawn fallito"));
		GameInstance->SetTurnMessage(TEXT("Errore: Spawn fallito"));
	}
}

ATile* ARandomPlayer::FindRandomValidTile(AGameField* GameField)
{
	if (!GameField)
	{
		UE_LOG(LogTemp, Error, TEXT("RandomPlayer: GameField null"));
		return nullptr;
	}

	// Array tile valide in Y=22,23,24
	TArray<ATile*> ValidTiles;

	// Cerco le tile valide tra Y=22-24, ovvero le tyle che sono camminabili e che non sono torri
	for (int32 Y = 22; Y <= 24; Y++)
	{
		for (int32 X = 0; X < GameField->GridSizeX; X++)
		{
			ATile* Tile = GameField->GetTileAtPosition(X, Y);

			if (Tile && Tile->IsWalkable() && Tile->GetTileType() != ETileType::TOWER)
			{
				// Devo controllare quali sono le tile libere e che non sono occupate dall'unità già piazzata
				bool bTileOccupied = false;
				for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
				{
					AUnit* Unit = *It;
					if (Unit && Unit->IsAlive())
					{
						FVector2D UnitPos = Unit->GetCurrentGridPosition();
						int32 UnitX = FMath::RoundToInt(UnitPos.X);
						int32 UnitY = FMath::RoundToInt(UnitPos.Y);

						if (UnitX == X && UnitY == Y)
						{
							bTileOccupied = true;
							break;
						}
					}
				}
				// Se la tile non è occupata dalla prima unità allora la aggiungo all'array delle tile valide per il piazzamento
				if (!bTileOccupied)
				{
					ValidTiles.Add(Tile);
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Trovate %d tile valide in Y=22,23,24"), ValidTiles.Num());

	// Seleziono una tile random tra quelle nell'array dove piazzare l'unità
	if (ValidTiles.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, ValidTiles.Num() - 1);
		ATile* SelectedTile = ValidTiles[RandomIndex];

		FVector2D Pos = SelectedTile->GetGridPosition();
		UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Tile casuale selezionata: (%d, %d)"), FMath::RoundToInt(Pos.X), FMath::RoundToInt(Pos.Y));
		// Faccio il return della tile in PlaceUnitAutomatically per poi spawnare l'unità
		return SelectedTile;
	}

	UE_LOG(LogTemp, Error, TEXT("RandomPlayer: Nessuna tile valida trovata nella zona Y=22,23,24"));
	return nullptr;
}

// Implemento A* per trovare il percorso per arrivare alla destinazione
// Ho usato fonti wikipedia e RedBlobGames
TArray<FIntPoint> ARandomPlayer::FindPathAStar(FIntPoint Start, FIntPoint Goal, AGameField* GameField)
{
	if (!GameField) return TArray<FIntPoint>();

	// Definisco la struttura per un generico nodo A*
	struct FPathNode
	{
		// Posizione nodo (X,Y)
		FIntPoint Position;
		// Costo per arrivare al nodo dal punto di partenza
		int32 GCost;
		// Quanta distanza manca per arrivare alla destinazione
		int32 HCost;
		// Costo totale stimato (somma di G e H)
		int32 FCost;
		// Punto da dove sono arrivatro per poter ricostruire il percorso
		FIntPoint Parent;

		FPathNode() : Position(0, 0), GCost(0), HCost(0), FCost(0), Parent(-1, -1) {}
		FPathNode(FIntPoint Pos, int32 G, int32 H, FIntPoint Par)
			: Position(Pos), GCost(G), HCost(H), FCost(G + H), Parent(Par) { }
	};

	// Lista nodi da esplorare
	TArray<FPathNode> OpenList;
	// Lista nodi già esplorati
	TArray<FIntPoint> ClosedList;
	// Tutti i nodi, così tengo traccia dei nodi parent già visitati
	TMap<FIntPoint, FPathNode> AllNodes;

	// Aggiungo il nodo iniziale
	int32 StartH = FMath::Abs(Goal.X - Start.X) + FMath::Abs(Goal.Y - Start.Y);
	FPathNode StartNode(Start, 0, StartH, FIntPoint(-1, -1));
	OpenList.Add(StartNode);
	AllNodes.Add(Start, StartNode);

	// Loop principale
	while (OpenList.Num() > 0)
	{
		// Trovo il nodo con FCost più basso
		int32 BestIndex = 0;
		for (int32 i = 1; i < OpenList.Num(); i++)
		{
			// Devo però considerare il caso in cui FCost sia uguale, allora scelgo quello con HCost più basso
			if (OpenList[i].FCost < OpenList[BestIndex].FCost ||
				(OpenList[i].FCost == OpenList[BestIndex].FCost && 
					OpenList[i].HCost < OpenList[BestIndex].HCost))
			{
				// Salvo l'indice del nodo migliore
				BestIndex = i;
			}
		}

		// Salvo la posizione corrispondente al nodo trovato aggiungendolo alla ClosedList
		FPathNode Current = OpenList[BestIndex];
		OpenList.RemoveAt(BestIndex);
		ClosedList.Add(Current.Position);

		// Se la tile corrente è il goal vuol dire che sono arrivato a destinazione
		if (Current.Position == Goal)
		{
			// Se sono arrivato alla destinazione allora ricostruisco il percorso
			// Devo inizializzare un array vuoto Path per salvare il percorso partendo dal Goal e tornando alla tile di partenza (Start)
			TArray<FIntPoint> Path;
			FIntPoint Step = Goal;

			// Siccome devo tornare a Start uso while e metto (-1,-1) che sarebbe il parent di Start, ovvero nessun parent perché è la partenza
			// così una volta raggiunto (-1,-1) esce dal while
			while (Step.X != -1 && Step.Y != -1)
			{
				// Metto Step nell'array del percorso (il primo step sarà la destinazione come da inizializzazione)
				Path.Insert(Step, 0);

				// Trovo il parent di Step
				if (AllNodes.Contains(Step))
				{
					// Salvo in Step il nodo corrente, in questo modo torno indietro fino ad arrivare a Start
					Step = AllNodes[Step].Parent;
				}
				else
				{
					// Se non trovo il parent esco dal while
					break;
				}
			}

			// Faccio return del percorso trovato
			UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Percorso A-Star trovato con %d step"), Path.Num());
			return Path;
		}

		// Guardo le tile adiacenti
		TArray<FIntPoint> Neighbors = {
			FIntPoint(Current.Position.X + 1, Current.Position.Y),
			FIntPoint(Current.Position.X - 1, Current.Position.Y),
			FIntPoint(Current.Position.X, Current.Position.Y + 1),
			FIntPoint(Current.Position.X, Current.Position.Y - 1)
		};

		// Per ogni tile adiacente trovata devo effettuare i soliti controlli
		for (const FIntPoint& Neighbor : Neighbors)
		{
			// Se sono dentro il GameField
			if (Neighbor.X < 0 || Neighbor.X >= GameField->GridSizeX ||
				Neighbor.Y < 0 || Neighbor.Y >= GameField->GridSizeY)
			{
				continue;
			}

			// Se è già nella lista ClosedList, ovvero nodi già visitati
			if (ClosedList.Contains(Neighbor))
			{
				continue;
			}

			// Se è una tile non camminabile
			ATile* NeighborTile = GameField->GetTileAtPosition(Neighbor.X, Neighbor.Y);
			if (!NeighborTile || !NeighborTile->IsWalkable())
			{
				continue;
			}

			// Dopo i controlli posso calcolare il nuovo GCost e HCost
			int32 NewGCost = Current.GCost + 1; // aggiungo +1 devo considerare questa nuova tile che percorro
			int32 HCost = FMath::Abs(Goal.X - Neighbor.X) + FMath::Abs(Goal.Y - Neighbor.Y);

			// Controllo se è nella OpenList, ovvero nodi ancora da esplorare
			bool bInOpenList = false;
			for (int32 i = 0; i < OpenList.Num(); i++)
			{
				if (OpenList[i].Position == Neighbor)
				{
					// Se c'è metto true
					bInOpenList = true;
					if (NewGCost < OpenList[i].GCost)
					{
						// Aggiorno il nodo perché ho trovato GCost migliore
						// Quindi update nuovo GCost, ricalcolo FCost, aggiorno il parent con la posizione corrente del nodo e infine salvo nella mappa AllNodes
						OpenList[i].GCost = NewGCost;
						OpenList[i].FCost = NewGCost + OpenList[i].HCost;
						OpenList[i].Parent = Current.Position;
						AllNodes[Neighbor] = OpenList[i];
					}
					// Allora posso uscire dal for
					break;
				}
			}

			// Se non è nella OpenList allora lo creo e lo aggiungo nella lista (sia OpenList sia nella mappa AllNodes)
			if (!bInOpenList)
			{
				FPathNode NewNode(Neighbor, NewGCost, HCost, Current.Position);
				OpenList.Add(NewNode);
				AllNodes.Add(Neighbor, NewNode);
			}
		}
	}

	// Nessun percorso trovato
	UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessun percorso trovato da (%d,%d) a (%d,%d)"),
		Start.X, Start.Y, Goal.X, Goal.Y);
	return TArray<FIntPoint>();
}

AUnit* ARandomPlayer::FindClosestEnemy()
{
	// Inizializzo qual è il ClosestEnemy come nullptr e la distanza minima che la setto al massimo, intanto poi la sovrascrivo e la uso nell'if
	AUnit* ClosestEnemy = nullptr;
	int32 MinDistance = INT_MAX;

	// Trovo qual è il centro delle unità dell'AI
	FVector2D AiCenter(0, 0);
	int32 AiUnitCount = 0;
	// Se ho ad esempio Sniper in (5,22) e Brawler in (12,23) allora MyCenter sarà (5+7,22+23) con MyUnitCount=2 a fine for
	for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
	{
		AUnit* Unit = *It;
		if (Unit && Unit->OwnerPlayerID == PlayerID && Unit->IsAlive())
		{
			AiCenter += Unit->GetCurrentGridPosition();
			AiUnitCount++;
		}
	}
	// Controllo per errore nel caso non ci fosserò unità
	if (AiUnitCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessuna unita' viva"));
		return nullptr;
	}
	// Qui siccome voglio trovare la posizione media divido AiCenter per il numero di unità
	AiCenter /= AiUnitCount;

	// Trova il nemico più vicino a AiCenter
	// Quello che faccio è considerare tutti gli actor che non sono di RandomPlayer e sono vivi
	for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
	{
		AUnit* Enemy = *It;
		if (Enemy && Enemy->OwnerPlayerID != PlayerID && Enemy->IsAlive())
		{
			// Se li trovo prendo la loro posizione sulla griglia
			FVector2D EnemyPos = Enemy->GetCurrentGridPosition();
			// Poi calcolo la distanza Manhattan (come ho fatto per la funzione sopra per il path) dall'AiCenter
			int32 Distance = FMath::Abs(EnemyPos.X - AiCenter.X) + FMath::Abs(EnemyPos.Y - AiCenter.Y);
			// Se la Distance è minore di MinDistance allora setto come ClosestEnemy il nemico che sto considerando
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				ClosestEnemy = Enemy;
			}
		}
	}
	// Se lo trovo salvo posizione
	if (ClosestEnemy)
	{
		FVector2D Pos = ClosestEnemy->GetCurrentGridPosition();
		UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Nemico più vicino trovato a (%d, %d), distanza: %d"), FMath::RoundToInt(Pos.X), FMath::RoundToInt(Pos.Y), MinDistance);
	}
	// Altrimenti mando log e l'Ai deve passare il turno
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessun nemico trovato"));
	}
	// Faccio return del nemico
	return ClosestEnemy;
}

ATile* ARandomPlayer::FindClosestFreeTower(AGameField* GameField)
{
	// Uso lo stesso ragionamento di FindClosestEnemy ma metto le torri libere: calcolo AiCenter e poi trovo torre libera
	if (!GameField) return nullptr;

	// Inizializzo ClosestTower e MinDistance
	ATile* ClosestTower = nullptr;
	int32 MinDistance = INT_MAX;

	// Trovo qual è il centro delle unità dell'AI
	FVector2D AiCenter(0, 0);
	int32 AiUnitCount = 0;

	for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
	{
		AUnit* Unit = *It;
		if (Unit && Unit->OwnerPlayerID == PlayerID && Unit->IsAlive())
		{
			AiCenter += Unit->GetCurrentGridPosition();
			AiUnitCount++;
		}
	}
	// Controllo per errore nel caso non ci fosserò unità
	if (AiUnitCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessuna unita' viva"));
		return nullptr;
	}
	// Qui siccome voglio trovare la posizione media divido AiCenter per il numero di unità
	AiCenter /= AiUnitCount;

	// Cerco tutte le torri presenti nella griglia
	TArray<ATower*> Towers = GameField->GetTowers();

	// Per ogni torre
	for (ATower* Tower : Towers)
	{
		if (!Tower) continue;

		// Considero solo le torri neutrali, contesa o sotto controllo dall'AI
		ETowerStatus TowerState = Tower->GetCurrentStatus();
		int32 TowerOwner = Tower->GetControllingPlayerID();

		// Se la torre è controllata da HumanPlayer allora non la considero
		if (TowerState == ETowerStatus::CONTROLLED && TowerOwner == 0)
		{
			continue;
		}

		// Se è libera, contesa o sotto controllo dell'AI allora calcolo distanza da AiCenter
		FVector2D TowerPos = Tower->GetGridPosition();
		// Calcolo la distanza Manhattan (come ho fatto per la funzione sopra per il path) dall'AiCenter
		int32 Distance = FMath::Abs(TowerPos.X - AiCenter.X) + FMath::Abs(TowerPos.Y - AiCenter.Y);
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			ClosestTower = GameField->GetTileAtPosition(FMath::RoundToInt(TowerPos.X), FMath::RoundToInt(TowerPos.Y));
		}
	}

	// Se la trovo salvo posizione
	if (ClosestTower)
	{
		FVector2D Pos = ClosestTower->GetGridPosition();
		UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Torre libera più vicina trovata a (%d, %d), distanza: %d"), FMath::RoundToInt(Pos.X), FMath::RoundToInt(Pos.Y), MinDistance);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessuna torre libera trovata"));
	}

	return ClosestTower;
}

FIntPoint ARandomPlayer::DecideTarget(AUnit* Unit, AGameField* GameField)
{
	// In questa funzione decido cosa fa l'AI in base alla priorità
	// Siccome da specifiche:
	// "L'AI deve valutare non solo la distanza dal nemico, ma anche la Priorità Torri, se una torre è libera e vicina l'AI deve tentare di conquistarla"
	if (!Unit || !GameField) return FIntPoint(-1, -1);

	FVector2D UnitPos = Unit->GetCurrentGridPosition();

	// Cerco la torre libera più vicina chiamando la funzione
	ATile* ClosestTower = FindClosestFreeTower(GameField);
	int32 DistanceToTower = INT_MAX;
	FIntPoint TowerCaptureTarget = FIntPoint(-1, -1);

	if (ClosestTower)
	{
		// Salvo la distanza
		FVector2D TowerPos = ClosestTower->GetGridPosition();
		DistanceToTower = FMath::Abs(TowerPos.X - UnitPos.X) + FMath::Abs(TowerPos.Y - UnitPos.Y);

		// Devo trovare una tile caminabile nella zona di cattura
		TowerCaptureTarget = FindWalkableTileCaptureZone(TowerPos, GameField);
		// Se non trovo nessuna tile camminabile allora la torre non è valida
		if (TowerCaptureTarget.X == -1)
		{
			DistanceToTower = INT_MAX;
		}
	}

	// Cerco il nemico più vicino chiamando la funzione
	AUnit* ClosestEnemy = FindClosestEnemy();
	int32 DistanceToEnemy = INT_MAX;

	if (ClosestEnemy)
	{
		// Salvo la distanza
		FVector2D EnemyPos = ClosestEnemy->GetCurrentGridPosition();
		DistanceToEnemy = FMath::Abs(EnemyPos.X - UnitPos.X) + FMath::Abs(EnemyPos.Y - UnitPos.Y);
	}

	// Ora faccio il confronto fra le distanze, se la distanza dalla torre è minore della distanza dal nemico allora vado dalla torre

	// Se c'è una torre più vicina del nemico vado dalla torre
	if (ClosestTower && DistanceToTower < DistanceToEnemy)
	{
		FVector2D TowerPos = ClosestTower->GetGridPosition();

		UE_LOG(LogTemp, Log, TEXT("RandomPlayer: %s priorita' TORRE a (%d, %d), distanza: %d (nemico: %d)"),
			*Unit->GetName(),
			FMath::RoundToInt(TowerPos.X),
			FMath::RoundToInt(TowerPos.Y),
			DistanceToTower,
			DistanceToEnemy);
		// Return della posizione della torre
		return TowerCaptureTarget;
	}
	// Altrimenti devo considerare il nemico
	else if (ClosestEnemy)
	{
		FVector2D EnemyPos = ClosestEnemy->GetCurrentGridPosition();

		UE_LOG(LogTemp, Log, TEXT("RandomPlayer: %s priorita' NEMICO a (%d, %d), distanza: %d (torre: %d)"),
			*Unit->GetName(),
			FMath::RoundToInt(EnemyPos.X),
			FMath::RoundToInt(EnemyPos.Y),
			DistanceToEnemy,
			DistanceToTower == INT_MAX ? -1 : DistanceToTower);
		// Return della posizione del nemico
		return FIntPoint(FMath::RoundToInt(EnemyPos.X), FMath::RoundToInt(EnemyPos.Y));
	}

	// Se non ci sono target disponibili faccio il return a (-1,-1)
	UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessun target disponibile per %s"), *Unit->GetName());
	return FIntPoint(-1, -1);
}

FIntPoint ARandomPlayer::FindWalkableTileCaptureZone(FVector2D TowerPos, AGameField* GameField)
{
	if (!GameField) return FIntPoint(-1, -1);

	// Prendo separate X e Y siccome per zona di cattura devo considerare due tile adiacenti alla torre (anche diagonale)
	int32 TowerX = FMath::RoundToInt(TowerPos.X);
	int32 TowerY = FMath::RoundToInt(TowerPos.Y);

	// Creo un array con le posizioni delle tile da controllare intorno alla torre
	TArray<FIntPoint> CaptureZoneTiles = {
		// Distanza 1 in tutte le direzioni
		FIntPoint(TowerX + 1, TowerY),
		FIntPoint(TowerX - 1, TowerY),
		FIntPoint(TowerX, TowerY + 1),
		FIntPoint(TowerX, TowerY - 1),
		// Distanza 2 in tute le direzioni + diagonali
		FIntPoint(TowerX + 2, TowerY),
		FIntPoint(TowerX - 2, TowerY),
		FIntPoint(TowerX, TowerY + 2),
		FIntPoint(TowerX, TowerY - 2),
		// Diagonali
		FIntPoint(TowerX + 1, TowerY + 1),
		FIntPoint(TowerX + 1, TowerY - 1),
		FIntPoint(TowerX - 1, TowerY + 1),
		FIntPoint(TowerX - 1, TowerY - 1)
	};

	// Nell'array parto controllando la prima tile e tutte le altre finché non ne trovo una camminabile. Quando la trovo faccio return della posizione
	for (const FIntPoint& CaptureZoneTile : CaptureZoneTiles)
	{
		// Se sono nella griglia allora vado avanti con la funzione altrrimenti esco e vado a quella dopo
		if (CaptureZoneTile.X < 0 || CaptureZoneTile.X >= GameField->GridSizeX ||
			CaptureZoneTile.Y < 0 || CaptureZoneTile.Y >= GameField->GridSizeY)
		{
			continue;
		}
		// Prendo la tile e verifico che sia camminabile
		ATile* Tile = GameField->GetTileAtPosition(CaptureZoneTile.X, CaptureZoneTile.Y);
		if (Tile && Tile->IsWalkable())
		{
			// Se lo è faccio return
			return CaptureZoneTile;
		}
	}
	// Se non trovo nessuna tile camminabile faccio log+return (-1,-1)
	UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessuna tile camminabile trovata vicino alla torre in (%d, %d)"), TowerX, TowerY);
	return FIntPoint(-1, -1);
}

FIntPoint ARandomPlayer::NextMoveTowardsTarget(AUnit* Unit, FIntPoint TargetPos, AGameField* GameField)
{
	if (!Unit || !GameField) return FIntPoint(-1, -1);

	// Salvo posizione corrente dell'unità
	FVector2D CurrentPos = Unit->GetCurrentGridPosition();
	FIntPoint Start(FMath::RoundToInt(CurrentPos.X), FMath::RoundToInt(CurrentPos.Y));

	// Se il target non è valido allora restituisco errore
	if (TargetPos.X == -1 || TargetPos.Y == -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Errore target non valido"));
		return FIntPoint(-1, -1);
	}

	// Chiamo la funzione FindPathAStar per usare A* per trovare il percorso verso TargetPos
	TArray<FIntPoint> Path = FindPathAStar(Start, TargetPos, GameField);
	// Se il path == 0 vuol dire che non è stato trovato nessun percorso
	if (Path.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessun percorso trovato per %s verso (%d, %d)"), *Unit->GetName(), TargetPos.X, TargetPos.Y);
		return FIntPoint(-1, -1);
	}

	// NOTA: il path include la posizione corrente salvato dell'unità come primo elemento, DEVO RIMUOVERLA
	// Rimuovo in index 0
	if (Path.Num() > 0 && Path[0] == Start)
	{
		Path.RemoveAt(0);
	}
	// Se dopo aver rimosso il primo elemento ho 0 vuol dire che sono già su TargetPos
	if (Path.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: %s e' già sul target"), *Unit->GetName());
		// Faccio return di Start
		return Start;
	}

	// Trovo le tile che sono raggiungibili chiamando la funzione GetReachableTiles
	TArray<FIntPoint> ReachableTiles = Unit->GetReachableTiles(GameField);
	// Trovo la tile che è più lontana nell'array delle tile raggiungibili
	// Inizializzo BestMove che è la tile migliore che l'AI può raggiungere ovvero quella più lontana raggiungibile
	FIntPoint BestMove = Start;
	// Inizializzo l'indice relativo alla BestMove
	int32 BestIndex = -1;
	// Per ogni indice in Path devo salvare la tile che è più lontana raggiungibile
	for (int32 i = 0; i < Path.Num(); i++)
	{
		if (ReachableTiles.Contains(Path[i]))
		{
			// Salvo BestMove e BestIndex
			BestMove = Path[i];
			BestIndex = i;
		}
		else
		{
			// Se non è raggiungibile esco dal for e tengo la tile salvata
			break;
		}
	}
	
	// Se BestIndex è ancora -1 vuol dire che non è stata trovata nessuna tile raggiungibile in path
	// Questo è il caso in cui nel percorso ci sono i seguenti ostacoli: unità nemiche (quelle di HumanPlayer) e le torri
	if (BestIndex == -1)
	{
		// Nessuna tile del percorso è raggiungibile, muovi verso la prima del percorso
		UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Non si puo' raggiungere il percorso A-star"))
		// Trovo la tile più vicina alla prima tile in Path che è raggiungibile
		FIntPoint FirstPathTile = Path[0];
		int32 MinDistance = INT_MAX;
		FIntPoint ClosestReachable = Start;

		// Faccio il for e applico lo stesso procedimento con Distance e MinDistance
		for (const FIntPoint& Reachable : ReachableTiles)
		{
			int32 Distance = FMath::Abs(FirstPathTile.X - Reachable.X) + FMath::Abs(FirstPathTile.Y - Reachable.Y);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				ClosestReachable = Reachable;
			}
		}
		// Scansionate tutte salvo come BestMove la tile più vicina che è raggiungibile senza ostacoli
		BestMove = ClosestReachable;
	}

	UE_LOG(LogTemp, Log, TEXT("RandomPlayer: %s muove verso (%d, %d), target finale: (%d, %d)"),
		*Unit->GetName(),
		BestMove.X, BestMove.Y,
		TargetPos.X, TargetPos.Y);

	return BestMove;
}

void ARandomPlayer::ShowMovementRange(AUnit* Unit, AGameField* GameField)
{
	if (!Unit || !GameField) return;

	// Se c'è un range di movimento attivo prima lo disattivo
	HideMovementRange(GameField);

	UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Mostro range movimento"));

	// Evidenzio la tile dell'unità prima di mostrare il range
	FVector2D UnitPos = Unit->GetCurrentGridPosition();
	CurrentUnitTile = GameField->GetTileAtPosition(FMath::RoundToInt(UnitPos.X), FMath::RoundToInt(UnitPos.Y));
	if (CurrentUnitTile)
	{
		FLinearColor AiColor = FLinearColor(0.7f, 0.0f, 1.0f, 1.0f);
		CurrentUnitTile->HighlightTile(true, AiColor);
	}

	// Creo l'array delle tile raggiungibili dall'unità
	TArray<FIntPoint> ReachableTiles = Unit->GetReachableTiles(GameField);
	// Evidenzio queste tile con la funzione ShowMovementOverlay per ogni tile dell'array
	for (const FIntPoint& TilePos : ReachableTiles)
	{
		ATile* Tile = GameField->GetTileAtPosition(TilePos.X, TilePos.Y);
		if (Tile)
		{
			Tile->ShowMovementOverlay(true);
			HighlightedMovementTiles.Add(Tile);
		}
	}
}

void ARandomPlayer::HideMovementRange(AGameField* GameField)
{
	// Rimuovo l'highlight della tile dell'unità
	if (CurrentUnitTile && IsValid(CurrentUnitTile))
	{
		CurrentUnitTile->HighlightTile(false);
		CurrentUnitTile = nullptr;
	}

	// Se non ci sono tile evidenziate non faccio niente
	if (HighlightedMovementTiles.Num() == 0) return;

	UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Nascondo range movimento"));
	// Per ogni tile evidenziata rimuovo l'overlay
	for (ATile* Tile : HighlightedMovementTiles)
	{
		if (Tile && IsValid(Tile))
		{
			Tile->ShowMovementOverlay(false);
		}
	}

	HighlightedMovementTiles.Empty();
}

// ProcessUnit è una funzione ricorsiva: voglio effettuare la ricorsione per poter mettere i timer tra le unità e lo ShowMovementOverlay
void ARandomPlayer::ProcessUnit(TArray<AUnit*> Units, int32 CurrentIndex, ATBS_GameMode* GM)
{
	if (!GM || !GM->GField|| GM->bGameEnded) return;

	// Se tutte le unità sono già state considerate faccio finire il turno
	if (CurrentIndex >= Units.Num())
	{
		if (GameInstance)
		{
			GameInstance->SetTurnMessage(TEXT("Turno AI completato"));
		}

		// Nascondo range movimento
		HideMovementRange(GM->GField);

		// Passo al turno di HumanPlayer dopo 1,5 secondi
		FTimerHandle EndTurnTimer;
		GetWorld()->GetTimerManager().SetTimer(EndTurnTimer, [this, GM]()
			{
				// Se la partita è finita o non c'è il gamemode return
				if (!GM || GM->bGameEnded) return;
				GM->TurnNextPlayer(PlayerID);
			}, 1.5f, false);
		return;
	}

	// Per ogni unità in AiUnits mostro il suo range di movimento
	AUnit* Unit = Units[CurrentIndex];
	UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Considero unità %s"), *Unit->GetName());
	// Mostro range movimento
	ShowMovementRange(Unit, GM->GField);
	// Mostro icone di attacco
	ShowAttackIndicators(Unit, GM->GField);

	// Timer 1 secondo e poi faccio eseguire azione all'AI
	FTimerHandle ActionTimer;
	GetWorld()->GetTimerManager().SetTimer(ActionTimer, [this, Unit, Units, CurrentIndex, GM]()
		{
			// Se la partita è finita o non c'è il gamemode return
			if (!GM || GM->bGameEnded) return;
			// DecideTarget per decidere se andare verso torre o nemico
			FIntPoint TargetPos = DecideTarget(Unit, GM->GField);
			// Se la posizione del target rimane (-1,-1) vuol dire che non sto considerando nessun target
			if (TargetPos.X == -1 || TargetPos.Y == -1)
			{
				UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessun target per %s"), *Unit->GetName());

				// Nascondo range movimento
				HideMovementRange(GM->GField);
				// Nascondo icone di attacco
				HideAttackIndicators(GM->GField);
				// Passo alla prossima unità
				ProcessUnit(Units, CurrentIndex + 1, GM);
				return;
			}

			// Calcolo la prossima mossa per arrivare al target usando algoritmo A* (ricordo che in NextMoveTowardsTarget chiamo FindPathAStar)
			FIntPoint NextMove = NextMoveTowardsTarget(Unit, TargetPos, GM->GField);
			// Se la posizione del target rimane (-1,-1) vuol dire che non c'è nessuna mossa valida verso il target
			if (NextMove.X == -1 || NextMove.Y == -1)
			{
				UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessuna mossa valida per %s"), *Unit->GetName());

				// Nascondo range movimento
				HideMovementRange(GM->GField);
				// Nascondo icone di attacco
				HideAttackIndicators(GM->GField);
				// Passo alla prossima unità
				ProcessUnit(Units, CurrentIndex + 1, GM);
				return;
			}

			// Muovo l'unità
			FVector2D CurrentPos = Unit->GetCurrentGridPosition();
			// Converto la posizione corrente dell'unità per confrontarla
			FIntPoint CurrentIntPos(FMath::RoundToInt(CurrentPos.X), FMath::RoundToInt(CurrentPos.Y));
			// Se non sono già sul target muovo l'unità
			if (NextMove != CurrentIntPos)
			{
				if (Unit->CanMoveTo(NextMove.X, NextMove.Y, GM->GField))
				{
					// Salvo la posizione prima del movimento per entry storico mosse
					FVector2D OldPos = Unit->GetCurrentGridPosition();

					Unit->MoveTo(NextMove.X, NextMove.Y, GM->GField);

					// Mostro icone di attacco dopo movimento
					ShowAttackIndicators(Unit, GM->GField);

					// Registro la mossa nello storico
					if (GameInstance)
					{
						FString MoveHistoryPlayerID = TEXT("AI");
						FString MoveHistoryUnitType = (Unit->UnitType == EUnitType::SNIPER) ? TEXT("S") : TEXT("B");
						FString MoveHistoryFromPos = AUnit::GridPositionConverter(FMath::RoundToInt(OldPos.X), FMath::RoundToInt(OldPos.Y));
						FString MoveHistoryToPos = AUnit::GridPositionConverter(NextMove.X, NextMove.Y);
						// Faccio il setup della stringa da passare poi allo storico delle mosse
						FString MoveEntry = FString::Printf(TEXT("%s: %s %s -> %s"), *MoveHistoryPlayerID, *MoveHistoryUnitType, *MoveHistoryFromPos, *MoveHistoryToPos);
						// Aggiungo la MoveEntry nell'array
						GameInstance->AddMoveToHistory(MoveEntry);
					}

					GameInstance->SetTurnMessage(FString::Printf(TEXT("AI: %s si muove"), *Unit->GetName()));
				}
			}

			// Se un nemico è in range l'AI lo attacca
			// Salvo in una rray i nemici da attaccare, controllo se è attaccabile con la funzione CanAttack
			TArray<AUnit*> EnemiesInRange;
			for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
			{
				AUnit* Enemy = *It;
				if (Enemy && Enemy->OwnerPlayerID != PlayerID && Enemy->IsAlive())
				{
					if (Unit->CanAttack(Enemy, GM->GField))
					{
						EnemiesInRange.Add(Enemy);
					}
				}
			}
			if (EnemiesInRange.Num() > 0)
			{
				// Attacco il nemico con meno punti vita
				AUnit* Target = EnemiesInRange[0];
				for (AUnit* Enemy : EnemiesInRange)
				{
					if (Enemy->CurrentHealth < Target->CurrentHealth)
					{
						Target = Enemy;
					}
				}
				// Calcolo il danno e poi lo applico
				int32 Damage = Unit->CalculateDamage();
				Target->ApplyDamage(Damage, Unit, GM->GField);

				// Registro la mossa nello storico (per l'attacco la entry la chiamo AttackEntry così so distinguere movimento e attacco se fatti nello stesso turno)
				if (GameInstance)
				{
					FString MoveHistoryPlayerID = TEXT("AI");
					FString MoveHistoryUnitType = (Unit->UnitType == EUnitType::SNIPER) ? TEXT("S") : TEXT("B");
					FVector2D MoveHistoryTargetPos = Target->GetCurrentGridPosition();
					FString MoveHistoryTargetPosConverted = AUnit::GridPositionConverter(FMath::RoundToInt(MoveHistoryTargetPos.X), FMath::RoundToInt(MoveHistoryTargetPos.Y));
					// Faccio il setup della stringa da passare poi allo storico delle mosse
					FString AttackEntry = FString::Printf(TEXT("%s: %s %s %d"), *MoveHistoryPlayerID, *MoveHistoryUnitType, *MoveHistoryTargetPosConverted, Damage);
					// Aggiungo la AttackEntry nell'array
					GameInstance->AddMoveToHistory(AttackEntry);
				}

				GameInstance->SetTurnMessage(FString::Printf(TEXT("AI: %s attacca!"), *Unit->GetName()));
			}
			else if (NextMove == CurrentIntPos)
			{
				// Se sono già sul target, ovvero la torre, se non ci sono nemici vicini l'AI starebbe ferma, allora calcolo il nemico più vicino e faccio muovere l'AI verso di lui
				UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: %s sul target ma nessun nemico in range, muovo verso nemico vicino"), *Unit->GetName());

				// Calcolo il nemico più vicino chiamando FindClosestEnemy
				AUnit* ClosestEnemy = FindClosestEnemy();
				if (ClosestEnemy)
				{
					FVector2D EnemyPos = ClosestEnemy->GetCurrentGridPosition();
					FIntPoint EnemyTarget(FMath::RoundToInt(EnemyPos.X), FMath::RoundToInt(EnemyPos.Y));
					// Definisco FallbackMove che rappresenta la prossima mossa per arrivare al nemico più vicino usando NextMoveTowardsTarget
					FIntPoint FallbackMove = NextMoveTowardsTarget(Unit, EnemyTarget, GM->GField);
					// Check per FallbackMove: deve essere diverso dalla posizione corrente dell'unità e deve essere nel range di movimento dell'unità
					if (FallbackMove != CurrentIntPos && Unit->CanMoveTo(FallbackMove.X, FallbackMove.Y, GM->GField))
					{
						// Muovo l'unità
						FVector2D OldPos = Unit->GetCurrentGridPosition();
						Unit->MoveTo(FallbackMove.X, FallbackMove.Y, GM->GField);

						// Registro la mossa nello storico
						if (GameInstance)
						{
							FString MoveHistoryPlayerID = TEXT("AI");
							FString MoveHistoryUnitType = (Unit->UnitType == EUnitType::SNIPER) ? TEXT("S") : TEXT("B");
							FString MoveHistoryFromPos = AUnit::GridPositionConverter(FMath::RoundToInt(OldPos.X), FMath::RoundToInt(OldPos.Y));
							FString MoveHistoryToPos = AUnit::GridPositionConverter(FallbackMove.X, FallbackMove.Y);
							// Faccio il setup della stringa da passare poi allo storico delle mosse
							FString MoveEntry = FString::Printf(TEXT("%s: %s %s -> %s"), *MoveHistoryPlayerID, *MoveHistoryUnitType, *MoveHistoryFromPos, *MoveHistoryToPos);
							// Aggiungo la AttackEntry nell'array
							GameInstance->AddMoveToHistory(MoveEntry);
						}

						GameInstance->SetTurnMessage(FString::Printf(TEXT("AI: %s si muove"), *Unit->GetName()));
					}
				}
			}

			// Aspetto 1 secondo e poi passo alla seconda unità
			FTimerHandle NextUnitTimer;
			GetWorld()->GetTimerManager().SetTimer(NextUnitTimer, [this, Units, CurrentIndex, GM]()
				{
					// Se la partita è finita o non c'è il gamemode return
					if (!GM || GM->bGameEnded) return;
					HideMovementRange(GM->GField);
					HideAttackIndicators(GM->GField);
					ProcessUnit(Units, CurrentIndex + 1, GM);
				}, 1.0f, false);

		}, 1.0f, false);
}

void ARandomPlayer::ShowAttackIndicators(AUnit* Unit, AGameField* GameField)
{
	if (!Unit || !GameField) return;

	// Nascondo le icone di attacco precedenti
	HideAttackIndicators(GameField);

	UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Mostro icone attacco"));

	// Trovo le unità nemiche attaccabili e mostro l'icona sopra di loro
	for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
	{
		AUnit* Enemy = *It;
		// Se l'unità è nemica ed è viva allora controllo se può essere attaccata dall'unità selezionata da RandomPlayer
		if (Enemy && Enemy->OwnerPlayerID != PlayerID && Enemy->IsAlive())
		{
			// Se l'unità selezionata può attaccare Enemy allora mostro l'icona
			if (Unit->CanAttack(Enemy, GameField))
			{
				// Spawno icona attacco sopra il target nemico
				if (AttackIndicatorClass)
				{
					FVector SpawnLoc = Enemy->GetActorLocation();

					AAttackIndicator* Indicator = GetWorld()->SpawnActor<AAttackIndicator>(
						AttackIndicatorClass,
						SpawnLoc,
						FRotator::ZeroRotator
					);

					if (Indicator)
					{
						Indicator->SetTargetUnit(Enemy);
						AttackIndicators.Add(Indicator);

						UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Icona attacco su %s"), *Enemy->GetName());
					}
				}
			}
		}
	}
}

void ARandomPlayer::HideAttackIndicators(AGameField* GameField)
{
	if (AttackIndicators.Num() == 0) return;

	UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Nascondo icone attacco che erano sui target"));

	// Distruggo tutte le icone nell'array
	for (AAttackIndicator* Indicator : AttackIndicators)
	{
		if (Indicator && IsValid(Indicator))
		{
			Indicator->Destroy();
		}
	}
	// Svuoto l'array per prossime icone attacco da aggiungere
	AttackIndicators.Empty();
}