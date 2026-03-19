// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit.h"
#include "GameField.h"
#include "TBS_GameMode.h"
#include "TBS_GameInstance.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AUnit::AUnit()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	UnitRoot = CreateDefaultSubobject<USceneComponent>(TEXT("UnitRoot"));
	RootComponent = UnitRoot;

	UnitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnitMesh"));
	UnitMesh->SetupAttachment(UnitRoot);
}

// Called when the game starts or when spawned
void AUnit::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Per ogni tick controllo se l'unità è in movimento
	if (bIsMoving)
	{
		MovementElapsedTime += DeltaTime;
		
		// Definisco Alpha come il rapporto tra il tempo trascorso e la durata totale del movimento
		// Rappresenta quanto sono lontano dalla posizione di partenza alla posizione di destinazione con valori tra 0.0f e 1.0f
		float Alpha = FMath::Clamp(MovementElapsedTime / MovementDuration, 0.0f, 1.0f);

		// Ora con funzione Lerp posso fare l'interpolazione del movimento tra posizione di partenza e posizione di destinazione, usando Alpha
		FVector NewLocation = FMath::Lerp(MovementStartLocation, MovementTargetLocation, Alpha);
		SetActorLocation(NewLocation);

		// Se Alpha è >= di 1.0f significa che sono arrivato alla posizione di destinazione, quindi fermo il movimento dell'unità e resetto tutte le variabili di movimento
		if (Alpha >= 1.0f)
		{
			// Siccome uso Clamp per limitare Alpha a 1.0f è possibile che esca un valore tipo 0.9
			// Allora setto l'unità nella posizione di destinazione
			SetActorLocation(MovementTargetLocation);

			// Incremento l'indice
			CurrentPathIndex++;

			if (CurrentPathIndex < MovementPath.Num())
			{
				// Siccome l'array non è finito continuo con la prossima tile nell'array
				// Ogni volta devo aggiornare la posizione di partenza, di destinazione e il tempo per arrivare alla posizione di destinazione
				MovementStartLocation = MovementTargetLocation;
				MovementTargetLocation = MovementPath[CurrentPathIndex];
				MovementElapsedTime = 0.0f;

				UE_LOG(LogTemp, Log, TEXT("Unit: Tile %d/%d raggiunta, continuo"), CurrentPathIndex, MovementPath.Num());
			}
			else
			{
				// Array finito, resetto tutte le variabili e metto bIsMoving a false per interrompere il movimento dell'unità
				bIsMoving = false;
				MovementElapsedTime = 0.0f;
				MovementPath.Empty();
				CurrentPathIndex = 0;

				UE_LOG(LogTemp, Log, TEXT("Unit: Movimento completato (path finito)"));
			}
		}
	}
}

// Called to bind functionality to input
void AUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// METODI MOVIMENTO

// Controllo se l'unità può muoversi alla target tile usando BFS e coda di tile esplorate 
// con controllo finale del costo totale del movimento per vedere se supera il movimento definito nelle proprietà dell'unità
bool AUnit::CanMoveTo(int32 TargetX, int32 TargetY, const AGameField* GameField) const
{
	if (!GameField) return false;

	// Tile di partenza
	FVector2D StartTile = GetCurrentGridPosition();
	int32 StartX = FMath::RoundToInt(StartTile.X);
	int32 StartY = FMath::RoundToInt(StartTile.Y);

	// Se tile corrente = target tile, non serve muoversi
	if (StartX == TargetX && StartY == TargetY) return false;

	// Altrimenti chiamo GetReachableTiles per ottenere tutte le tile raggiungibili con BFS
	TArray<FIntPoint> ReachableTiles = GetReachableTiles(GameField);

	// Controllo se la target tile è tra quelle raggiungibili
	for (const FIntPoint& Tile : ReachableTiles)
	{
		if (Tile.X == TargetX && Tile.Y == TargetY)
		{
			return true;
		}
	}

	// Se tile non raggiungibile ritorno false
	return false;
}

TArray<FIntPoint> AUnit::GetReachableTiles(const AGameField* GameField) const
{
	TArray<FIntPoint> ReachableTiles;

	if (!GameField) return ReachableTiles;

	// Tile di partenza
	FVector2D StartTile = GetCurrentGridPosition();
	int32 StartX = FMath::RoundToInt(StartTile.X);
	int32 StartY = FMath::RoundToInt(StartTile.Y);

	// Array per tenere traccia delle tile visitate
	TArray<bool> Visited;
	Visited.Init(false, GameField->GridSizeX * GameField->GridSizeY);

	// Coda per BFS
	TQueue<FIntPoint> Queue;
	TMap<FIntPoint, int32> CostMap;

	// Parto dalla StartTile
	FIntPoint StartPoint(StartX, StartY);
	Queue.Enqueue(StartPoint);
	Visited[StartY * GameField->GridSizeX + StartX] = true;
	CostMap.Add(StartPoint, 0);

	// Direzioni di movimento (no diagonali)
	TArray<FIntPoint> Directions = {
		FIntPoint(0, 1),
		FIntPoint(0, -1),
		FIntPoint(1, 0),
		FIntPoint(-1, 0)
	};

	// BFS per trovare tutte le tile raggiungibili
	while (!Queue.IsEmpty())
	{
		FIntPoint Current;
		Queue.Dequeue(Current);

		int32 CurrentCost = CostMap[Current];

		// Controllo le 4 tile adiacenti
		for (const FIntPoint& Dir : Directions)
		{
			int32 NeighborX = Current.X + Dir.X;
			int32 NeighborY = Current.Y + Dir.Y;

			// Controllo di essere dentro la griglia
			if (NeighborX >= 0 && NeighborX < GameField->GridSizeX && NeighborY >= 0 && NeighborY < GameField->GridSizeY)
			{
				int32 Index = NeighborY * GameField->GridSizeX + NeighborX;

				if (!Visited[Index])
				{
					ATile* Neighbor = GameField->GetTileAtPosition(NeighborX, NeighborY);

					if (Neighbor && Neighbor->IsWalkable())
					{
						int32 MoveCost = GetMovementCost(Current.X, Current.Y, NeighborX, NeighborY, GameField);

						if (MoveCost > 0)
						{
							int32 NewCost = CurrentCost + MoveCost;

							if (NewCost <= MaxMovement)
							{
								Visited[Index] = true;
								CostMap.Add(FIntPoint(NeighborX, NeighborY), NewCost);
								Queue.Enqueue(FIntPoint(NeighborX, NeighborY));

								// Aggiungo la tile nelle array delle tile raggiungibili
								ReachableTiles.Add(FIntPoint(NeighborX, NeighborY));
							}
						}
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GetReachableTiles: %d tile raggiungibili trovate"), ReachableTiles.Num());
	return ReachableTiles;
}

TArray<FIntPoint> AUnit::GetPathTo(int32 TargetX, int32 TargetY, const AGameField* GameField) const
{
	TArray<FIntPoint> Path;

	if (!GameField) return Path;

	// Tile di partenza
	FVector2D StartTile = GetCurrentGridPosition();
	int32 StartX = FMath::RoundToInt(StartTile.X);
	int32 StartY = FMath::RoundToInt(StartTile.Y);

	// Se sono già alla target tile faccio return dell'array Path che è vuoto
	if (StartX == TargetX && StartY == TargetY) return Path;

	// Uso BFS con ricostruzione del percorso
	TArray<bool> Visited;
	Visited.Init(false, GameField->GridSizeX * GameField->GridSizeY);

	// Coda per BFS e mappe per poi ricostruire il percorso
	TQueue<FIntPoint> Queue;
	TMap<FIntPoint, FIntPoint> CameFromMap;
	TMap<FIntPoint, int32> CostMap;

	// Parto dalla StartTile
	FIntPoint StartPoint(StartX, StartY);
	Queue.Enqueue(StartPoint);
	Visited[StartY * GameField->GridSizeX + StartX] = true;
	CostMap.Add(StartPoint, 0);

	// Direzioni di movimento (no diagonali)
	TArray<FIntPoint> Directions = {
		FIntPoint(0, 1),
		FIntPoint(0, -1),
		FIntPoint(1, 0),
		FIntPoint(-1, 0)
	};

	// Variabile bool per tenere traccia se ho trovato la target tile
	bool bFoundTarget = false;

	// Inizio BFS
	while (!Queue.IsEmpty() && !bFoundTarget)
	{
		FIntPoint Current;
		Queue.Dequeue(Current);

		// Se ho trovato il target esco dal ciclo e metto bool a true
		if (Current.X == TargetX && Current.Y == TargetY)
		{
			bFoundTarget = true;
			break;
		}

		int32 CurrentCost = CostMap[Current];

		// Controllo le 4 tile adiacenti
		for (const FIntPoint& Dir : Directions)
		{
			int32 NeighborX = Current.X + Dir.X;
			int32 NeighborY = Current.Y + Dir.Y;

			// Controllo di essere dentro la griglia
			if (NeighborX >= 0 && NeighborX < GameField->GridSizeX && NeighborY >= 0 && NeighborY < GameField->GridSizeY)
			{
				int32 Index = NeighborY * GameField->GridSizeX + NeighborX;

				if (!Visited[Index])
				{
					ATile* Neighbor = GameField->GetTileAtPosition(NeighborX, NeighborY);

					if (Neighbor && Neighbor->IsWalkable())
					{
						int32 MoveCost = GetMovementCost(Current.X, Current.Y, NeighborX, NeighborY, GameField);

						if (MoveCost > 0)
						{
							int32 NewCost = CurrentCost + MoveCost;

							if (NewCost <= MaxMovement)
							{
								Visited[Index] = true;
								CostMap.Add(FIntPoint(NeighborX, NeighborY), NewCost);
								// Aggiungo la tile nelle array per ricostruire il percorso di movimento
								CameFromMap.Add(FIntPoint(NeighborX, NeighborY), Current);
								Queue.Enqueue(FIntPoint(NeighborX, NeighborY));
							}
						}
					}
				}
			}
		}
	}

	// Ricostruisco ora il percorso al contrario pasrtendo dalla target tile usando CameFromMap fino ad arrivare alla start tile
	if (bFoundTarget)
	{
		FIntPoint Current(TargetX, TargetY);

		while (!(Current.X == StartX && Current.Y == StartY))
		{
			// Inserisco la tile corrente all'inizio dell'array Path, in questo modo prendo il percorso salvato in CameFromMap e cambio ordine
			Path.Insert(Current, 0);
			Current = CameFromMap[Current];
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GetPathTo: Percorso calcolato con %d tile"), Path.Num());
	return Path;
}

bool AUnit::MoveTo(int32 TargetX, int32 TargetY, AGameField* GameField)
{
	// Controllo se posso muovermi alla target tile
	if (!CanMoveTo(TargetX, TargetY, GameField))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveTo fallito: tile non raggiungibile"));
		return false;
	}

	// Calcolo il percorso alla target tile chiamando GetPathTo
	TArray<FIntPoint> PathTiles = GetPathTo(TargetX, TargetY, GameField);
	// Se array vuoto non c'è un percorso valido e faccio return false
	if (PathTiles.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("MoveTo: Path non trovato"));
		return false;
	}

	// Ora prendo PathTiles e lo converto in una array con le posizioni reali delle tile nel gamefield
	// In questo modo posso usare questo array per far muovere le unità con l'interpolazione del movimento nel Tick
	MovementPath.Empty();
	// Per ogni tile in PathTiles
	for (const FIntPoint& TilePos : PathTiles)
	{
		ATile* PathTile = GameField->GetTileAtPosition(TilePos.X, TilePos.Y);
		if (PathTile)
		{
			FVector TileLocation = PathTile->GetActorLocation();
			TileLocation.Z += 150.f;
			MovementPath.Add(TileLocation);
		}
	}

	// Aggiorno la posizione corrente dell'unità
	SetCurrentGridPosition(FVector2D(TargetX, TargetY));

	// Se ci sono tile in MovementPath allora faccio partire l'interpolazione del movimento mettendo bIsMoving a true e far partire il movimento nel Tick
	if (MovementPath.Num() > 0)
	{
		MovementStartLocation = GetActorLocation();
		MovementTargetLocation = MovementPath[0];
		CurrentPathIndex = 0;
		MovementElapsedTime = 0.0f;
		bIsMoving = true;
	}

	return true;
}

// Calcolo del costo di movimento in base alla differenza di livello tra tile corrente e target tile
int32 AUnit::GetMovementCost(int32 FromX, int32 FromY, int32 TargetX, int32 TargetY, const AGameField* GameField) const
{
	if (!GameField) return 0;

	// Tile corrente e target tile
	FVector2D CurrPos = GetCurrentGridPosition();
	ATile* CurrTile = GameField->GetTileAtPosition(FromX, FromY);
	ATile* TargetTile = GameField->GetTileAtPosition(TargetX, TargetY);

	if (!TargetTile || !CurrTile) return 0;

	// Non è possibile muoversi sopra le caselle di livello 0 (acqua), Torri e dove è già presente un'altra unità
	// anche se queste caselle si trovano nel tragitto (non si possono "saltare)
	if (!TargetTile->IsWalkable())
	{
		return 0;
	}

	// Controllo se sulla target tile c'è già un'altra unità
	for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
	{
		AUnit* OtherUnit = *It;
		if (OtherUnit && OtherUnit != this && OtherUnit->IsAlive())
		{
			FVector2D OtherPos = OtherUnit->GetCurrentGridPosition();
			if (FMath::RoundToInt(OtherPos.X) == TargetX && FMath::RoundToInt(OtherPos.Y) == TargetY)
			{
				// C'è già un'altra unità sulla target tile
				UE_LOG(LogTemp, Log, TEXT("GetMovementCost: Tile (%d, %d) occupata da %s"), TargetX, TargetY, *OtherUnit->GetName());
				return 0;
			}
		}
	}

	// Ora calcolo il costo del movimento in base al livello delle tile
	int32 CurrLevelHeight = CurrTile->GetHeightLevel();
	int32 TargetLevelHeight = TargetTile->GetHeightLevel();
	int32 LevelDifference = TargetLevelHeight - CurrLevelHeight;

	// Stesso livello, costo 1
	if (LevelDifference == 0) {
		return 1;
	}
	// Salita di un livello, costo 2
	else if (LevelDifference == 1) {
		return 2;
	}
	// Discesa di un livello, costo 1
	else if (LevelDifference == -1) {
		return 1;
	}
	// Altro non permesso
	else {
		return 0;
	}
}

// Respawn dell'unità alla posizione iniziale
void AUnit::RespawnAtInitialPosition()
{
	if (!IsValid(this)) return;

	// Resetto la vita al massimo di nuovo
	CurrentHealth = MaxHealth;
	// Aggiorno gli HP nell'UI
	if (UTBS_GameInstance* GI = Cast<UTBS_GameInstance>(GetWorld()->GetGameInstance()))
	{
		bool bIsSniper = (UnitType == EUnitType::SNIPER);
		GI->UpdateUnitHP(OwnerPlayerID, bIsSniper, CurrentHealth);
	}
	// Metto l'unità alla posizione iniziale di piazzamento
	SetCurrentGridPosition(InitialGridPosition);

	// Ottengo la tile di spawn iniziale dal GameField, appena la trovo esco dal for
	AGameField* GameField = nullptr;
	for (TActorIterator<AGameField> It(GetWorld()); It; ++It)
	{
		GameField = *It;
		break;
	}
	// Definisco la RespawnTile
	if (GameField)
	{
		ATile* RespawnTile = GameField->GetTileAtPosition(FMath::RoundToInt(InitialGridPosition.X),	FMath::RoundToInt(InitialGridPosition.Y));
		// Se esiste devo piazzare l'unità sulla tile
		if (RespawnTile)
		{
			// Faccio il respawn dell'unità sulla tile di piazzamento iniziale
			FVector TileLocation = RespawnTile->GetActorLocation();
			FVector NewLocation = TileLocation;
			NewLocation.Z = TileLocation.Z + 150.f;
			// Siccome è un respawn automatico non attivo l'interpolazione ma piazzo direttamente l'unità sulla tile
			SetActorLocation(NewLocation);
			bIsMoving = false;
			MovementElapsedTime = 0.0f;
			// Rendo l'unità di nuovo visibile e riattivo collisioni
			SetActorHiddenInGame(false);
			SetActorEnableCollision(true);
			// Resetto il turno dell'unità
			bHasMovedThisTurn = false;
			bHasAttackedThisTurn = false;

			UE_LOG(LogTemp, Log, TEXT("RespawnAtInitialPosition: %s respawnato a (%.0f, %.0f) con %d HP"), *GetName(), InitialGridPosition.X, InitialGridPosition.Y,CurrentHealth);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RespawnAtInitialPosition: Tile di respawn non trovata"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RespawnAtInitialPosition: GameField non trovato"));
	}
}

// METODI ATTACCO
// Calcolo del danno in base al range MinDamage e MaxDamage
int32 AUnit::CalculateDamage() const
{
	// Calcolo il valore del danno randomicamente dal range MinDamage e MaxDamage (valore intero)
	int32 Damage = FMath::RandRange(MinDamage, MaxDamage);

	// Log
	UE_LOG(LogTemp, Log, TEXT("AUnit::CalculateDamage - Danno calcolato: %d"), Damage);

	return Damage;
}

// Sottrazione punti vita dopo attacco subito
void AUnit::ApplyDamage(int32 DamageAmount, AUnit* Attacker, const AGameField* GameField)
{
	if (DamageAmount <= 0) return;

	// Riproduco AttackSound assegnato nel blueprint dell'unità che attacca, se assegnato
	if (Attacker && Attacker->AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Attacker->AttackSound, Attacker->GetActorLocation());
	}

	// Sottraggo danno inflito dalla vita corrente e faccio in modo che non scenda sotto 0
	CurrentHealth -= DamageAmount;
	CurrentHealth = FMath::Max(0, CurrentHealth);

	if (UTBS_GameInstance* GI = Cast<UTBS_GameInstance>(GetWorld()->GetGameInstance()))
	{
		bool bIsSniper = (UnitType == EUnitType::SNIPER);
		GI->UpdateUnitHP(OwnerPlayerID, bIsSniper, CurrentHealth);
	}

	// Se l'unità è ancora viva dopo l'attacco allora può effettuare il danno da contrattacco
	// Il controattacco però è possibile se l'unità che deve eseguire il controattacco è viva
	if (CurrentHealth > 0 && Attacker && GameField)
	{
		CounterAttack(Attacker, GameField);
	}

	// Controllo se l'unità è morta
	if (CurrentHealth <= 0)
	{
		// Riproduco DeathSound assegnato nel blueprint dell'unità uccisa, se assegnato
		if (DeathSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
		}

		// Unità distrutta, imposto vita a 0
		CurrentHealth = 0;

		// Nascondo l'unità e disabilito collisioni
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);

		// Notifica GameMode per gestire respawn
		if (ATBS_GameMode* GM = GetWorld()->GetAuthGameMode<ATBS_GameMode>())
		{
			GM->OnUnitDied(this);
		}
	}
}

bool AUnit::CanAttack(AUnit* TargetUnit, const AGameField* GameField) const
{
	// Controlli base sul targetunit
	if (!TargetUnit || !TargetUnit->IsAlive() || TargetUnit->OwnerPlayerID == OwnerPlayerID)
	{
		UE_LOG(LogTemp, Log, TEXT("CanAttack fallito: la TargetUnit è non valida, morta o è l'unità dell'OwnerPlayerID"));
		return false;
	}

	// Posizione attuale e targetpos
	FVector2D MyPos = GetCurrentGridPosition();
	FVector2D TargetPos = TargetUnit->GetCurrentGridPosition();

	// Calcolo distanza tra target e tile attuale
	int32 DistX = FMath::Abs(FMath::RoundToInt(TargetPos.X - MyPos.X));
	int32 DistY = FMath::Abs(FMath::RoundToInt(TargetPos.Y - MyPos.Y));
	int32 Distance = FMath::Max(DistX, DistY);

	// Controllo range massimo di attacco e distanza 0 (in questo caso attacco fallito
	if (Distance > AttackRange || Distance == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("CanAttack fallito: distanza attacco oltre range o distanza=0"));
		return false;
	}

	// Cerco le tile per controllare l'altezza
	ATile* MyTile = GameField->GetTileAtPosition(FMath::RoundToInt(MyPos.X), FMath::RoundToInt(MyPos.Y));
	ATile* TargetTile = GameField->GetTileAtPosition(FMath::RoundToInt(TargetPos.X), FMath::RoundToInt(TargetPos.Y));

	if (!MyTile || !TargetTile)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanAttack: tile non trovata"));
		return false;
	}

	// Prendo l'altezza dove mi trovo e della target unit
	int32 MyLevel = MyTile->GetHeightLevel();
	int32 TargetLevel = TargetTile->GetHeightLevel();

	// Altezza target deve essere stesso livello o inferiore
	if (TargetLevel > MyLevel)
	{
		UE_LOG(LogTemp, Log, TEXT("CanAttack fallito: target livello %d > mio %d"), TargetLevel, MyLevel);
		return false;
	}

	// Regole per RANGED
	if (AttackType == EAttackType::RANGED)
	{
		// Lo sniper può oltrepassare acqua
		UE_LOG(LogTemp, Log, TEXT("CanAttack: Sniper può attaccare"));
		return true;
	}
	else // Rengole per MELEE
	{
		// Il brawler può attaccare solo in tile adiacenti e con distanza=1
		if (Distance != 1)
		{
			UE_LOG(LogTemp, Log, TEXT("CanAttack fallito: distanza oltre 1"));
			return false;
		}

		// Check se targettile camminabile
		if (!TargetTile->IsWalkable())
		{
			UE_LOG(LogTemp, Log, TEXT("CanAttack fallito: acqua"));
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("CanAttack: Brawler può attaccare"));
		return true;
	}
}

void AUnit::CounterAttack(AUnit* Attacker, const AGameField* GameField)
{
	if (!Attacker || !Attacker->IsAlive() || !IsAlive() || !GameField) return;

	// Deve essere uno sniper quindi se non lo è faccio return
	if (Attacker->UnitType != EUnitType::SNIPER) return;

	// L'unità attaccata può fare il contrattacco se è uno sniper oppure se è un brawler e lo sniper è a distanza 1
	bool CanCounter = false;

	if (UnitType == EUnitType::SNIPER)
	{
		// Sniper vs sniper contrattacca sempre
		CanCounter = true;
	}
	else if (UnitType == EUnitType::BRAWLER)
	{
		// Il brawler può attaccare se è a distanza 1
		FVector2D UnitPos = GetCurrentGridPosition();
		FVector2D AttackerPos = Attacker->GetCurrentGridPosition();

		int32 DistX = FMath::Abs(FMath::RoundToInt(AttackerPos.X - UnitPos.X));
		int32 DistY = FMath::Abs(FMath::RoundToInt(AttackerPos.Y - UnitPos.Y));
		int32 Distance = FMath::Max(DistX, DistY);

		if (Distance <= 1)
		{
			CanCounter = true;
		}
	}
	// Se il bool è false allora faccio return perché non è possibile fare il contrattacco
	if (!CanCounter) return;

	// Ora calcolo i danni del contrattacco: range da 1 a 3
	int32 CounterDamage = FMath::RandRange(1, 3);
	// Applico il danno
	Attacker->ApplyDamage(CounterDamage, nullptr, GameField);
}

bool AUnit::IsAlive() const
{
	// L'unità è viva se ha almeno 1 punto vita, altrimenti metto no
	bool Alive = (CurrentHealth > 0);

	return Alive;
}

// METODI TURNI
// A fine turno resetto le due variabili di stato per permettere all'unità di muoversi e attaccare di nuovo nel prossimo turno
void AUnit::ResetTurnStatus()
{
	bHasMovedThisTurn = false;
	bHasAttackedThisTurn = false;
}

FString AUnit::GridPositionConverter(int32 X, int32 Y)
{
	FString Letter;
	// Per ogni X da 0 a 25 assegno una lettera da A a Z aggiungendo ad "A" il valore di X usando la funzione FString::Chr
	if (X >= 0 && X < 26)
	{
		Letter = FString::Chr('A' + X);
	}

	// Per il numero è sufficiente prendere dirramente Y e poi convertirlo in stringa nel return
	int32 Number = Y;

	// Combino lette e numero per ottenere il formato A0
	return FString::Printf(TEXT("%s%d"), *Letter, Number);
}
