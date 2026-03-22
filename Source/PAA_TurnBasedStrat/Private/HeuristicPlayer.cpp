// Fill out your copyright notice in the Description page of Project Settings.


#include "HeuristicPlayer.h"
#include "GameField.h"
#include "TBS_GameMode.h"
#include "Unit.h"
#include "Tile.h"
#include "Tower.h"
#include "EngineUtils.h"

// Sets default values
AHeuristicPlayer::AHeuristicPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GameInstance = Cast<UTBS_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	// Inizializzo valori default
	// Inizializzo valori default
	PlayerID = 1;
	ClassToSpawn = nullptr;
	IsMyTurn = false;
	bHasPlacedSniper = false;
	bHasPlacedBrawler = false;
	CurrentUnitTile = nullptr;
}

// Called when the game starts or when spawned
void AHeuristicPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void AHeuristicPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// ANNULLA IL TIMER!
	// Questo impedisce che la funzione venga eseguita dopo che il mondo è stato distrutto.
	GetWorld()->GetTimerManager().ClearTimer(AI_TurnTimerHandle);
}

// Called every frame
void AHeuristicPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AHeuristicPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AHeuristicPlayer::OnPlacementTurnStart()
{
	UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Turno di piazzamento iniziato"));
	IsMyTurn = true;

	// Messaggio che AI sta piazzando
	if (GameInstance)
	{
		GameInstance->SetTurnMessage(TEXT("L'AI sta piazzando..."));
	}

	// Piazzamento automatico dopo il timer di 1.5f
	GetWorld()->GetTimerManager().SetTimer(AI_TurnTimerHandle, this, &AHeuristicPlayer::PlaceUnitAutomatically, 1.5f, false);
}

void AHeuristicPlayer::OnTurnStart()
{
	IsMyTurn = true;
	UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Turno AI"));
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
	GetWorld()->GetTimerManager().SetTimer(AI_TurnTimerHandle, this, &AHeuristicPlayer::PerformTurnActions, 1.5f, false);
}

void AHeuristicPlayer::OnTurnEnd()
{
	IsMyTurn = false;
	UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Turno terminato"));
}

void AHeuristicPlayer::PerformTurnActions()
{
	ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GField || GM->bGameEnded) return;

	UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Inizio esecuzione azioni dell'AI"));

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

	// Chiamo ProcessUnit per ogni unità dell'AI
	ProcessUnit(AiUnits, 0, GM);
}

void AHeuristicPlayer::OnWin()
{
	GameInstance->SetTurnMessage(TEXT("L'AI ha vinto la partita!"));

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	HideMovementRange(GetWorld()->GetAuthGameMode<ATBS_GameMode>()->GField);
	HideAttackIndicators(GetWorld()->GetAuthGameMode<ATBS_GameMode>()->GField);
}

void AHeuristicPlayer::OnLose()
{
	GameInstance->SetTurnMessage(TEXT("L'AI ha perso la partita!"));

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	HideMovementRange(GetWorld()->GetAuthGameMode<ATBS_GameMode>()->GField);
	HideAttackIndicators(GetWorld()->GetAuthGameMode<ATBS_GameMode>()->GField);
}

void AHeuristicPlayer::PlaceUnitAutomatically()
{
	ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode());
	if (!GM)
	{
		UE_LOG(LogTemp, Error, TEXT("HeuristicPlayer: GameMode non trovato!"));
		return;
	}

	if (!GM->bPlacementPhase)
	{
		UE_LOG(LogTemp, Error, TEXT("HeuristicPlayer: Non è più la fase di piazzamento!"));
		return;
	}

	if (!IsMyTurn)
	{
		UE_LOG(LogTemp, Error, TEXT("HeuristicPlayer: Non è il mio turno!"));
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

		UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Scelta unità dell'AI: %s"), *UnitName);
	}
	// Una volta piazzata la prima unità devo spawnare la seconda
	else if (!bHasPlacedSniper)
	{
		// Caso sniper
		ClassToSpawn = SniperClass;
		UnitName = TEXT("Sniper");
		bIsSniper = true;
		UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Secondo turno, Sniper piazzato"));
	}
	else if (!bHasPlacedBrawler)
	{
		// Caso brawler
		ClassToSpawn = BrawlerClass;
		UnitName = TEXT("Brawler");
		bIsSniper = false;
		UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Secondo turno, Brawler piazzato"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HeuristicPlayer: Tutte le unità già piazzate!"));
		return;
	}

	// Verifico che la classe da spawnare sia assegnata correttamente
	if (!ClassToSpawn)
	{
		UE_LOG(LogTemp, Error, TEXT("HeuristicPlayer: ClassToSpawn è null"));
		GameInstance->SetTurnMessage(TEXT("Errrore: Classe unità non assegnata"));
		return;
	}

	// Ora devo trovare una tile valida casuale in Y=22,23,24
	ATile* TargetTile = FindRandomValidTile(GM->GField);

	if (!TargetTile)
	{
		UE_LOG(LogTemp, Error, TEXT("HeuristicPlayer: Nessuna tile valida trovata"));
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
		}
		else
		{
			bHasPlacedBrawler = true;
		}

		int32 X = FMath::RoundToInt(TilePos.X);
		int32 Y = FMath::RoundToInt(TilePos.Y);
		UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: %s piazzato in (%d, %d)"), *UnitName, X, Y);
		GameInstance->SetTurnMessage(FString::Printf(TEXT("L'AI ha piazzato %s"), *UnitName));

		// Notifico GameMode che l'unità è stata piazzata
		GM->OnUnitPlaced(1);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HeuristicPlayer: Spawn fallito"));
		GameInstance->SetTurnMessage(TEXT("Errore: Spawn fallito"));
	}
}

ATile* AHeuristicPlayer::FindRandomValidTile(AGameField* GameField)
{
	if (!GameField)
	{
		UE_LOG(LogTemp, Error, TEXT("HeuristicPlayer: GameField null"));
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

	UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Trovate %d tile valide in Y=22,23,24"), ValidTiles.Num());

	// Seleziono una tile random tra quelle nell'array dove piazzare l'unità
	if (ValidTiles.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, ValidTiles.Num() - 1);
		ATile* SelectedTile = ValidTiles[RandomIndex];

		FVector2D Pos = SelectedTile->GetGridPosition();
		UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Tile casuale selezionata: (%d, %d)"), FMath::RoundToInt(Pos.X), FMath::RoundToInt(Pos.Y));
		// Faccio il return della tile in PlaceUnitAutomatically per poi spawnare l'unità
		return SelectedTile;
	}

	UE_LOG(LogTemp, Error, TEXT("HeuristicPlayer: Nessuna tile valida trovata nella zona Y=22,23,24"));
	return nullptr;
}

TArray<FIntPoint> AHeuristicPlayer::FindPathGreedy(FIntPoint Start, FIntPoint Goal, AGameField* GameField)
{
	// Uso l'algoritmo Greedy Best First Search per trovare un percorso da Start a Goal, usando le funzioni euristiche per decidere quali nodi (quindi le tile) esplorare
	if (!GameField) return TArray<FIntPoint>();

	// La differenza da A* è che qui considero SOLO il costo euristico (indicato con HCost) per decidere quale tile esplorare
	struct FGreedyNode
	{
		FIntPoint Position;
		int32 HCost;
		FIntPoint Parent;

		FGreedyNode() : Position(0, 0), HCost(0), Parent(-1, -1) {}
		FGreedyNode(FIntPoint Pos, int32 H, FIntPoint Par)
			: Position(Pos), HCost(H), Parent(Par) {
		}
	};

	// Lista nodi da esplorare
	TArray<FGreedyNode> OpenList;
	// Lista nodi già esplorati
	TArray<FIntPoint> ClosedList;
	// Tutti i nodi, così tengo traccia dei nodi parent già visitati
	TMap<FIntPoint, FGreedyNode> AllNodes;

	// Aggiungo il nodo iniziale
	int32 StartH = ManhattanDistance(Start, Goal);
	FGreedyNode StartNode(Start, StartH, FIntPoint(-1, -1));
	OpenList.Add(StartNode);
	AllNodes.Add(Start, StartNode);

	// Loop principale
	while (OpenList.Num() > 0)
	{
		// Trovo il nodo con HCost più basso
		int32 BestIndex = 0;
		for (int32 i = 1; i < OpenList.Num(); i++)
		{
			if (OpenList[i].HCost < OpenList[BestIndex].HCost)
			{
				// Salvo l'indice del nodo migliore
				BestIndex = i;
			}
		}

		// Salvo la posizione corrispondente al nodo trovato aggiungendolo alla ClosedList
		FGreedyNode Current = OpenList[BestIndex];
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
			// Così una volta raggiunto (-1,-1) esce dal while
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
			UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Percorso Greedy trovato con %d step"), Path.Num());
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

			// Dopo i controlli posso calcolare il nuovo HCost
			int32 HCost = ManhattanDistance(Neighbor, Goal);

			// Controllo se è nella OpenList, ovvero nodi ancora da esplorare
			bool bInOpenList = false;
			for (int32 i = 0; i < OpenList.Num(); i++)
			{
				if (OpenList[i].Position == Neighbor)
				{
					// Se c'è metto true
					bInOpenList = true;
					// Faccio break perché non devo continuare a cercare e non devo assegnare il nuovo HCost
					break;
				}
			}

			// Se non è nella OpenList allora lo creo e lo aggiungo nella lista (sia OpenList sia nella mappa AllNodes)
			if (!bInOpenList)
			{
				FGreedyNode NewNode(Neighbor, HCost, Current.Position);
				OpenList.Add(NewNode);
				AllNodes.Add(Neighbor, NewNode);
			}
		}
	}

	// Nessun percorso trovato
	UE_LOG(LogTemp, Warning, TEXT("HeuristicPlayer: Nessun percorso trovato da (%d,%d) a (%d,%d)"),
		Start.X, Start.Y, Goal.X, Goal.Y);
	return TArray<FIntPoint>();
}

int32 AHeuristicPlayer::ManhattanDistance(FIntPoint A, FIntPoint B)
{
	// Siccome le unità si possono muovere solo in orizzontale e verticale creo questa funzione per richiamare il calcolo della distanza quando mi serve
	return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
}

ATower* AHeuristicPlayer::EvaluateBestTower(AUnit* Unit, AGameField* GameField)
{
	// Con questa funzione prendo tutte e 3 le torri presenti nel gamefield e per ognuna chiamo EvaluateTowerScore per calcolare il loro score
	if (!Unit || !GameField) return nullptr;

	// Salvo la posizione dell'unità selezionata
	FVector2D UnitPos = Unit->GetCurrentGridPosition();
	// Prendo tutte le torri nel gamefield
	TArray<ATower*> Towers = GameField->GetTowers();
	// Inizializzo le variabili che userò per salvare la torre con lo score più alto
	ATower* BestTower = nullptr;
	float BestScore = -100.0f;

	// Per ogni torre
	for (ATower* Tower : Towers)
	{
		if (!Tower) continue;

		// Calcolo lo score per la torre
		float TowerScore = EvaluateTowerScore(Tower, UnitPos);
		// Se TowerScore è più alto di BestScore aggiorno BestTower e BestScore
		if (TowerScore > BestScore)
		{
			BestTower = Tower;
			BestScore = TowerScore;
		}
	}

	// Se dopo il for ho BestTower non null allora ho trovato la torre con lo score più alto da raggiungere
	if (BestTower)
	{
		// Faccio log
		FVector2D BestTowerPos = BestTower->GetGridPosition();
		UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Miglior torre selezionata: (%d,%d) con Score=%.1f"), FMath::RoundToInt(BestTowerPos.X), FMath::RoundToInt(BestTowerPos.Y), BestScore);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HeuristicPlayer: Nessuna torre valida trovata"));
	}

	// Return di BestTower
	return BestTower;
}

AUnit* AHeuristicPlayer::EvaluateBestTarget(AUnit* Attacker)
{
	// Con questa funzione prendo tutte le unità nemiche presenti nel gamefield e per ognuna chiamo EvaluateEnemyScore per calcolare il loro score
	if (!Attacker) return nullptr;

	// Salvo la posizione dell'attacker
	FVector2D AttackerPos = Attacker->GetCurrentGridPosition();
	// Inizializzo le variabili che userò per salvare il nemico con lo score più alto
	AUnit* BestTarget = nullptr;
	float BestScore = -100.0f;

	// Per ogni nemico
	for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
	{
		AUnit* Enemy = *It;

		// Continue se l'unità non è valida, è morta o non è dell'AI
		if (!Enemy || !Enemy->IsAlive()) continue;
		if (Enemy->OwnerPlayerID == PlayerID) continue;

		// Calcolo score per il nemico
		float EnemyScore = EvaluateEnemyScore(Enemy, AttackerPos);
		// Se EnemyScore è più alto di BestScore aggiorno BestTarget e BestScore
		if (EnemyScore > BestScore)
		{
			BestTarget = Enemy;
			BestScore = EnemyScore;
		}
	}

	// Se dopo il for ho BestTarget non null allora ho trovato l'unità nemica con lo score più alto da attaccare
	if (BestTarget)
	{
		FVector2D BestTargetPos = BestTarget->GetCurrentGridPosition();
		UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Miglior nemico: (%d,%d) HP=%d Score=%.1f"),	
			FMath::RoundToInt(BestTargetPos.X), FMath::RoundToInt(BestTargetPos.Y),	BestTarget->CurrentHealth, BestScore);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HeuristicPlayer: Nessun nemico valido da attaccare"));
	}
	// Return di BestTarget
	return BestTarget;
}

float AHeuristicPlayer::EvaluateTowerScore(ATower* Tower, FVector2D UnitPos)
{
	// Con questa funzione valuto la torre passata alla funzione e assegno un punteggio e ciascuna per dare una priorità all'AI su quale torre deve raggiungere
	// Il punteggio lo assegno in base allo status e alla distanza dall'unità
	// Status torre: Neutrale>Contesa>Sotto controllo nemico
	// Distanza: più è vicina e meglio è
	// Come output salvo uno score e scelgo quella con lo score più alto

	// Inizializzazioni
	float Score = 0.0f;
	ETowerStatus Status = Tower->GetCurrentStatus();
	int32 TowerOwner = Tower->GetControllingPlayerID();

	// Assegno lo score in base allo status della torre
	if (Status == ETowerStatus::NEUTRAL)
	{
		// Torre neutrale = priorità massima
		Score += 100.0f;
	}
	else if (Status == ETowerStatus::CONTROLLED && TowerOwner != PlayerID)
	{
		// Torre del nemico = bassa priorità ma fare contesa se è vicina
		Score += 80.0f;
	}
	else if (Status == ETowerStatus::CONTESTED)
	{
		// Torre contesa = da difendere o conquistare
		Score += 50.0f;
	}
	else if (Status == ETowerStatus::CONTROLLED && TowerOwner == PlayerID)
	{
		// Torre già dell'AI = da difendere ma dopo torre contesa/neutrale
		Score += 30.0f;
	}

	// Ora da questo score sottraggo un punteggio per la distanza, più è lontana più lo score diminuisce
	FVector2D TowerPos = Tower->GetGridPosition();
	FIntPoint TowerIntPos(FMath::RoundToInt(TowerPos.X), FMath::RoundToInt(TowerPos.Y));
	FIntPoint UnitIntPos(FMath::RoundToInt(UnitPos.X), FMath::RoundToInt(UnitPos.Y));

	// Calcolo la distanza tra unità e torre usando ManhattanDistance e sottraggo dallo score 2 punti per ogni tile di distanza
	int32 Distance = ManhattanDistance(UnitIntPos, TowerIntPos);
	Score -= Distance * 2.0f;

	UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Torre (%d,%d) Status=%d Owner=%d Distanza=%d → Score=%.1f"), TowerIntPos.X, TowerIntPos.Y, (int32)Status, TowerOwner, Distance, Score);
	// Return dello score
	return Score;
}

float AHeuristicPlayer::EvaluateEnemyScore(AUnit* Enemy, FVector2D UnitPos)
{
	// Con questa funzione valuto l'unità passata alla funzione e assegno un punteggio per decidere quale unità attaccare
	// Il punteggio lo assegno in base agli HP rimanenti, la classe (sniper più pericoloso) e alla distanza dall'unità
	// HP rimanenti: più sono bassi più è facile uccidere l'unità
	// Classe: lo sniper è più pericoloso del brawler
	// Distanza: più è vicina e meglio è
	// Come output salvo uno score e scelgo quella con lo score più alto

	// Inizializzazioni
	float Score = 0.0f;
	int32 MaxHP = (Enemy->UnitType == EUnitType::SNIPER) ? 20 : 40;
	float HPPercent = (float)Enemy->CurrentHealth / (float)MaxHP;

	// Più sono bassi gli HP più è alto lo score
	// HP 100%: 0 punti
	// HP 75%: 25 punti
	// HP 50%: 50 punti
	// HP 25%: 75 punti
	// HP 0%: 100 punti
	Score += (1.0f - HPPercent) * 100.0f;

	// Se è uno sniper aggiungo 20 punti allo score siccome l'attacco a distanza è più pericoloso
	if (Enemy->UnitType == EUnitType::SNIPER)
	{
		Score += 20.0f;
	}

	// Ora da questo score sottraggo un punteggio per la distanza, più è lontana più lo score diminuisce
	FVector2D EnemyPos = Enemy->GetCurrentGridPosition();
	FIntPoint EnemyIntPos(FMath::RoundToInt(EnemyPos.X), FMath::RoundToInt(EnemyPos.Y));
	FIntPoint UnitIntPos(FMath::RoundToInt(UnitPos.X), FMath::RoundToInt(UnitPos.Y));

	// Calcolo la distanza tra unità dell'AI e unità nemica usando ManhattanDistance e sottraggo dallo score 3 punti per ogni tile di distanza
	// La penalità deve essere più alta rispetto a quella delle torri perché è più importante attaccare una unità nemica vicina che raggiungere una torre lontana
	int32 Distance = ManhattanDistance(UnitIntPos, EnemyIntPos);
	Score -= Distance * 3.0f;

	UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Nemico (%d,%d) HP=%d/%d Classe=%d Distanza=%d Score=%.1f"),	EnemyIntPos.X, EnemyIntPos.Y, Enemy->CurrentHealth, MaxHP, (int32)Enemy->UnitType, Distance, Score);
	// Return dello score
	return Score;
}

FIntPoint AHeuristicPlayer::DecideTarget(AUnit* Unit, AGameField* GameField)
{
	if (!Unit || !GameField) return FIntPoint(-1, -1);

	// Valuto quale torre è più conveniente raggiungere o quale nemico è più conveniente attaccare utilizzando le funzione euristiche che ho definito
	FVector2D UnitPos = Unit->GetCurrentGridPosition();

	// Valuto la torre migliore (EvaluateBestTower)
	ATower* BestTower = EvaluateBestTower(Unit, GameField);
	float TowerScore;
	if (BestTower != nullptr)
	{
		TowerScore = EvaluateTowerScore(BestTower, UnitPos);
	}
	else
	{
		TowerScore = -100.0f;
	}

	// Valuto l'unità nemica migliore da attaccare (EvaluateBestTarget)
	AUnit* BestEnemy = EvaluateBestTarget(Unit);
	float EnemyScore;
	if (BestEnemy != nullptr)
	{
		EnemyScore = EvaluateEnemyScore(BestEnemy, UnitPos);
	}
	else
	{
		EnemyScore = -100.0f;
	}

	// Confronto gli score e decido se l'AI deve muoversi verso la torre o verso il nemico per attaccarlo
	if (TowerScore > EnemyScore && BestTower)
	{
		FVector2D TowerPos = BestTower->GetGridPosition();
		FIntPoint TowerTarget = FindWalkableTileCaptureZone(TowerPos, GameField);

		UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: %s priorità TORRE (Score: %.1f > %.1f)"), *Unit->GetDisplayName(), TowerScore, EnemyScore);

		return TowerTarget;
	}
	else if (BestEnemy)
	{
		FVector2D EnemyPos = BestEnemy->GetCurrentGridPosition();

		UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: %s priorità NEMICO (Score: %.1f > %.1f)"), *Unit->GetDisplayName(), EnemyScore, TowerScore);

		return FIntPoint(FMath::RoundToInt(EnemyPos.X), FMath::RoundToInt(EnemyPos.Y));
	}

	UE_LOG(LogTemp, Warning, TEXT("HeuristicPlayer: Nessun target disponibile per %s"), *Unit->GetDisplayName());
	return FIntPoint(-1, -1);
}

FIntPoint AHeuristicPlayer::FindWalkableTileCaptureZone(FVector2D TowerPos, AGameField* GameField)
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
	UE_LOG(LogTemp, Warning, TEXT("HeuristicPlayer: Nessuna tile camminabile trovata vicino alla torre in (%d, %d)"), TowerX, TowerY);
	return FIntPoint(-1, -1);
}

FIntPoint AHeuristicPlayer::NextMoveTowardsTarget(AUnit* Unit, FIntPoint TargetPos, AGameField* GameField)
{
	if (!Unit || !GameField) return FIntPoint(-1, -1);

	// Salvo posizione corrente dell'unità
	FVector2D CurrentPos = Unit->GetCurrentGridPosition();
	FIntPoint Start(FMath::RoundToInt(CurrentPos.X), FMath::RoundToInt(CurrentPos.Y));

	// Se il target non è valido allora restituisco errore
	if (TargetPos.X == -1 || TargetPos.Y == -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("HeuristicPlayer: Errore target non valido"));
		return FIntPoint(-1, -1);
	}

	// Chiamo la funzione FindPathGreedy per usare l'algoritmo per trovare il percorso verso TargetPos
	TArray<FIntPoint> Path = FindPathGreedy(Start, TargetPos, GameField);
	// Se il path == 0 vuol dire che non è stato trovato nessun percorso
	if (Path.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Nessun percorso trovato per %s verso (%d, %d)"), *Unit->GetDisplayName(), TargetPos.X, TargetPos.Y);
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
		UE_LOG(LogTemp, Warning, TEXT("HeuristicPlayer: %s e' già sul target"), *Unit->GetDisplayName());
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
		UE_LOG(LogTemp, Warning, TEXT("HeuristicPlayer: Non si puo' raggiungere il percorso Greedy calcolato"))
			// Trovo la tile più vicina alla prima tile in Path che è raggiungibile
			FIntPoint FirstPathTile = Path[0];
		int32 MinDistance = INT_MAX;
		FIntPoint ClosestReachable = Start;

		// Faccio il for e applico lo stesso procedimento con Distance e MinDistance
		for (const FIntPoint& Reachable : ReachableTiles)
		{
			int32 Distance = ManhattanDistance(FirstPathTile, Reachable);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				ClosestReachable = Reachable;
			}
		}
		// Scansionate tutte salvo come BestMove la tile più vicina che è raggiungibile senza ostacoli
		BestMove = ClosestReachable;
	}

	UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: %s muove verso (%d,%d), target finale: (%d,%d)"), *Unit->GetDisplayName(), BestMove.X, BestMove.Y, TargetPos.X, TargetPos.Y);
	// Return di BestMove calcolata ovvero la tile più lontana raggiungibile nel percorso calcolato verso TargetPos
	return BestMove;
}

void AHeuristicPlayer::ShowMovementRange(AUnit* Unit, AGameField* GameField)
{
	if (!Unit || !GameField) return;

	// Se c'è un range di movimento attivo prima lo disattivo
	HideMovementRange(GameField);

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

void AHeuristicPlayer::HideMovementRange(AGameField* GameField)
{
	// Rimuovo l'highlight della tile dell'unità
	if (CurrentUnitTile && IsValid(CurrentUnitTile))
	{
		CurrentUnitTile->HighlightTile(false);
		CurrentUnitTile = nullptr;
	}

	// Se non ci sono tile evidenziate non faccio niente
	if (HighlightedMovementTiles.Num() == 0) return;

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

void AHeuristicPlayer::ProcessUnit(TArray<AUnit*> Units, int32 CurrentIndex, ATBS_GameMode* GM)
{
	if (!GM || !GM->GField || GM->bGameEnded) return;

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
	UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Considero unità %s"), *Unit->GetDisplayName());
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
				UE_LOG(LogTemp, Warning, TEXT("HeuristicPlayer: Nessun target per %s"), *Unit->GetDisplayName());

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
				UE_LOG(LogTemp, Warning, TEXT("HeuristicPlayer: Nessuna mossa valida per %s"), *Unit->GetDisplayName());

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

					GameInstance->SetTurnMessage(FString::Printf(TEXT("%s si muove"), *Unit->GetDisplayName()));
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
				// Resetto il danno da contrattacco precedente
				Unit->LastCounterDamage = 0;
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
					FString AttackEntry;
					AttackEntry = FString::Printf(TEXT("%s: %s %s %d"), *MoveHistoryPlayerID, *MoveHistoryUnitType, *MoveHistoryTargetPosConverted, Damage);

					GameInstance->AddMoveToHistory(AttackEntry);

					if (Unit->LastCounterDamage > 0)
					{
						// Definisco chi effettua il contrattacco (ovvero AI)
						FString CounterPlayerID = TEXT("HP");
						// Unità che effettua il contrattacco
						FString CounterUnitType = (Target->UnitType == EUnitType::SNIPER) ? TEXT("S") : TEXT("B");
						// Unità che riceve il contrattacco
						FVector2D CounterTargetPos = Unit->GetCurrentGridPosition();
						FString CounterTargetPosConverted = AUnit::GridPositionConverter(FMath::RoundToInt(CounterTargetPos.X), FMath::RoundToInt(CounterTargetPos.Y));
						// Faccio il setup della stringa da passare poi allo storico delle mosse
						FString CounterAttackEntry;
						CounterAttackEntry = FString::Printf(TEXT("Counter: %s %s %s %d"), *CounterPlayerID, *CounterUnitType, *CounterTargetPosConverted, Unit->LastCounterDamage);

						GameInstance->AddMoveToHistory(CounterAttackEntry);
					}
				}

				GameInstance->SetTurnMessage(FString::Printf(TEXT("%s attacca!"), *Unit->GetDisplayName()));
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

void AHeuristicPlayer::ShowAttackIndicators(AUnit* Unit, AGameField* GameField)
{
	if (!Unit || !GameField) return;

	// Nascondo le icone di attacco precedenti
	HideAttackIndicators(GameField);

	UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Mostro icone attacco"));

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

						UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Icona attacco su %s"), *Enemy->GetDisplayName());
					}
				}
			}
		}
	}
}

void AHeuristicPlayer::HideAttackIndicators(AGameField* GameField)
{
	if (AttackIndicators.Num() == 0) return;

	UE_LOG(LogTemp, Log, TEXT("HeuristicPlayer: Nascondo icone attacco che erano sui target"));

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