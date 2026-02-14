// Fill out your copyright notice in the Description page of Project Settings.

#include "GameField.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Tile.h"
#include "Tower.h"
#include "Containers/Queue.h"

// Sets default values
AGameField::AGameField()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when actor is placed in the world or spawned
void AGameField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

// Called when the game starts or when spawned
void AGameField::BeginPlay()
{
	Super::BeginPlay();

	GenerateGrid();
	PlaceTowers();
}

// Generazione griglia con Perlin Noise
void AGameField::GenerateGrid()
{
	// Per specifiche tutte le tile devono essere raggiungibili e non ci devono essere isole o sezioni di mappa irraggiungibili
	// Per tentativi di generazione cerco di ottenere una mapppa connessa
	// Limito num tentativi a 30 per evitare loop infiniti, intanto 30 è un buono numero per ottenere una mappa connessa
	int32 Attempts = 0;
	const int32 MaxAttempts = 30;

	while (Attempts < MaxAttempts)
	{
		if (!GetWorld() || !TileClass)
		{
			UE_LOG(LogTemp, Error, TEXT("GameField: World or TileClass not valid"));
			return;
		}

		// Distruggo le tile della precedente generazione se ce ne sono e uso il for range based
		// for ( init-statement ﻿(optional) item-declaration : range-initializer ) statement
		for (ATile* OldTile : TileArray)
		{
			// Verifico che non sia nullptr e che non sia già stata distrutta
			if (OldTile && IsValid(OldTile))
			{
				UE_LOG(LogTemp, Log, TEXT("Distruggo vecchia tile siccome la mappa deve essere rigenerata"));
				OldTile->Destroy();
			}
		}

		TileArray.Empty();
		TileMap.Empty();

		// Seed random per generare mappe diverse ad ogni play
		const int32 Seed = FMath::Rand();
		FRandomStream RandomStream(Seed);

		// Scale factor per Perlin Noise (nota: più è basso più ho pianure)
		const float NoiseScale = 0.08f;

		// Offset randomici per evitare pattern simili
		const float OffsetX = RandomStream.FRandRange(0.f, 10000.f);
		const float OffsetY = RandomStream.FRandRange(0.f, 10000.f);

		for (int32 X = 0; X < GridSizeX; X++)
		{
			for (int32 Y = 0; Y < GridSizeY; Y++)
			{
				// Calcolo il Perlin Noise per la tile corrente. Nota: il PerlinNoise2D ritorna valori in [-1,1] quindi devo normalizzare dopo
				const float NoiseValue = FMath::PerlinNoise2D(
					FVector2D(
						(X + OffsetX) * NoiseScale,
						(Y + OffsetY) * NoiseScale
					)
				);

				// Normalizzo da [-1,1] a [0,1]
				float NormalizedNoise = (NoiseValue + 1.0f) * 0.5f;

				// Converto il NormalizedNoise in un livello di altezza discreto
				int32 HeightLevel;
				if (NormalizedNoise < 0.42f)
				{
					HeightLevel = 0;
				}
				else if (NormalizedNoise < 0.46f)
				{
					HeightLevel = 1;
				}
				else if (NormalizedNoise < 0.54f)
				{
					HeightLevel = 2;
				}
				else if (NormalizedNoise < 0.64f)
				{
					HeightLevel = 3;
				}
				else
				{
					HeightLevel = 4;
				}

				// Effettuo il clamp per sicurezza
				HeightLevel = FMath::Clamp(HeightLevel, 0, MaxHeight);

				// Spawno la tile nella pisizione X,Y e con l'altezza calcolata
				SpawnTile(X, Y, HeightLevel);
			}
		}

		// Dopo lo spawn delle tile controllo se la mappa è connessa ed è quindi senza isole irraggiungibili
		if (IsMapFullyConnected())
		{
			return;
		}
		// Altrimenti mappa non connessa, allora aumento il contatore dei tentativi e rigenero la mappa e riverifico se la mappa è interamente connessa
		UE_LOG(LogTemp, Warning, TEXT("Mappa non connessa - tentativo %d/%d"), Attempts + 1, MaxAttempts);
		Attempts++;
	}
	// Non sono riuscito a generare una mappa connessa dopo i tentativi massimi
	UE_LOG(LogTemp, Error, TEXT("Impossibile generare mappa connessa dopo %d tentativi"), MaxAttempts);
}

void AGameField::PlaceTowers()
{
	if (!TowerClass || TileMap.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameField: TowerClass non valida oppure mappa delle tile vuota"));
		return;
	}
	// Punti ideali per le torri
	TArray<FVector2D> TowerPositions = {
		FVector2D(12, 12),
		FVector2D(5, 12),
		FVector2D(19,12)
	};

	// Per ogni posizione ideale che ho definito cerco la posizione camminabile più vicina e spawno la torre
	for (const FVector2D& IdealPos : TowerPositions)
	{
		FVector2D BestPos = FindNearestWalkablePosition(IdealPos);

		int32 X = FMath::RoundToInt(BestPos.X);
		int32 Y = FMath::RoundToInt(BestPos.Y);
		
		ATile* TargetTile = GetTileAtPosition(X, Y);
		if (!TargetTile) continue;

		// Posizione di spawn della torre
		FVector SpawnLocation = TargetTile->GetActorLocation();

		// Spawno la torre
		ATower*	NewTower = GetWorld()->SpawnActor<ATower>(
			TowerClass,
			SpawnLocation,
			FRotator::ZeroRotator
		);

		if (NewTower)
		{
			// Imposto il tipo della tyle come tower
			TargetTile->SetTileType(ETileType::TOWER);

			// Log di conferma
			UE_LOG(LogTemp, Log, TEXT("Torre spawnata in (%d,%d)"), X, Y);
		}
	}
}

// Funzione per trovare la posizione walkable più vicina a IdealPos
FVector2D AGameField::FindNearestWalkablePosition(const FVector2D& IdealPos) const
{
	int32 IdealX = FMath::RoundToInt(IdealPos.X);
	int32 IdealY = FMath::RoundToInt(IdealPos.Y);

	// Controllo la posizione ideale
	if (ATile* Tile = GetTileAtPosition(IdealX, IdealY))
	{
		// Se la tile non è acqua allora ritorno la posizione ideale
		if (Tile->GetHeightLevel() >= 1)
		{
			return IdealPos;
		}
	}

	// Uso l'algoritmo di ricerca a spirale per trovare la tile che non sia water più vicina al punto ideale
	// stackoverflow.com/questions/3330181/algorithm-for-finding-nearest-object-on-2d-grid

	// Il raggio della spirale (quindi il cerchio di ricerca) suppongo che parta da 1 e arrivi max a 10
	// Siccome la mappa è 25x25 e ho utilizzato il Perlin Noise la probabilità di non trovare una tile camminabile in un raggio di 8 tiles immagino sia bassa
	for (int32 Radius = 1; Radius <= 10; ++Radius)
	{
		for (int32 dx = -Radius; dx <= Radius; ++dx)
		{
			for (int32 dy = -Radius; dy <= Radius; ++dy)
			{
				// Controllo i bordi del quadrato centrato in (IdealX, IdealY)
				// è sufficiente controllare solo i bordi esterni perché le posizioni interne ai bordi le ho controllate nei quadrati precedenti
				if (FMath::Abs(dx) == Radius || FMath::Abs(dy) == Radius)
				{
					// Caalcolo la posizione effettiva nella griglia
					int32 X = IdealX + dx;
					int32 Y = IdealY + dy;

					// Check per non uscire fuori dalla griglia
					if (X >= 0 && X < GridSizeX && Y >= 0 && Y < GridSizeY)
					{
						// Controllo se la tile in (X,Y) è camminabile (ovvero se non è acqua)
						if (ATile* Tile = GetTileAtPosition(X, Y))
						{
							if (Tile->GetHeightLevel() >= 1)
							{
								// Se la tile è camminabile allora faccio return della posizione in cui verrà piazzata la torre
								return FVector2D(X, Y);
							}
						}
					}
				}
			}
		}
	}

	// Nel caso estremo in cui non trovo tile nel raggio di 10 ritorno la posizione centrale (molto difficile che accada ma non si sa mai)
	UE_LOG(LogTemp, Warning, TEXT("Nessuna cella camminabile trovata. Spawno la torre al centro della griglia."));
	return FVector2D(12, 12);
}

// Funzione per verificare se le tile della mappa sono tutte raggiungibili/connesse
bool AGameField::IsMapFullyConnected() const
{
	// Trovo la prima tile camminabile
	ATile* Start = FindFirstWalkableTile();
	// Mappa vuota o nessuna tile camminabile (messo per sicurezza e completezza)
	if (!Start) return true;

	// Array per poter tenere traccia delle tile che ho già visitato
	TArray<bool> Visited;
	Visited.Init(false, GridSizeX * GridSizeY);

	int32 SX = FMath::RoundToInt(Start->GetGridPosition().X);
	int32 SY = FMath::RoundToInt(Start->GetGridPosition().Y);

	// Eseguo il flood fill dalla tile di partenza per segnare quali sono le tile che posso raggiungere
	FloodFill(SX, SY, Visited);

	// Controllo che tutte le tile che sono camminabili siano state visitate
	for (int32 X = 0; X < GridSizeX; ++X)
	{
		for (int32 Y = 0; Y < GridSizeY; ++Y)
		{
			ATile* Tile = GetTileAtPosition(X, Y);
			// Se la tile è camminabile e non è stata visitata allora la mappa non è completamente connessa
			if (Tile && Tile->IsWalkable() && !Visited[Y * GridSizeX + X])
			{
				// Tile non camminabile trovata allora metto false e devo rigenerare la mappa
				UE_LOG(LogTemp, Warning, TEXT("Mappa non completamente connessa. Cella non raggiungibile in (%d,%d)"), X, Y);
				return false;
			}
		}
	}
	return true;
}

void AGameField::FloodFill(int32 StartX, int32 StartY, TArray<bool>& Visited) const
{
	TQueue<FIntPoint> Queue;
	Queue.Enqueue(FIntPoint(StartX, StartY));
	Visited[StartY * GridSizeX + StartX] = true;

	// Solo 4 direzioni (no diagonali, come da specs movimento)
	TArray<FIntPoint> Directions = {
		FIntPoint(0, 1), FIntPoint(0, -1),
		FIntPoint(1, 0), FIntPoint(-1, 0)
	};

	while (!Queue.IsEmpty())
	{
		FIntPoint Current;
		Queue.Dequeue(Current);

		for (const FIntPoint& Dir : Directions)
		{
			int32 NX = Current.X + Dir.X;
			int32 NY = Current.Y + Dir.Y;

			if (NX >= 0 && NX < GridSizeX && NY >= 0 && NY < GridSizeY)
			{
				int32 Index = NY * GridSizeX + NX;
				if (!Visited[Index])
				{
					ATile* Neighbor = GetTileAtPosition(NX, NY);
					if (Neighbor && Neighbor->IsWalkable())
					{
						Visited[Index] = true;
						Queue.Enqueue(FIntPoint(NX, NY));
					}
				}
			}
		}
	}
}

ATile* AGameField::FindFirstWalkableTile() const
{
	for (const auto& Elem : TileMap)
	{
		ATile* Tile = Elem.Value;
		if (Tile && Tile->IsWalkable())
		{
			return Tile;
		}
	}
	return nullptr;
}

// Funzione per spawnare la tile in X,Y e con altezza HeightLevel
ATile* AGameField::SpawnTile(int32 X, int32 Y, int32 HeightLevel)
{
	const FVector SpawnLocation(
		X * TileSpacing,
		Y * TileSpacing,
		0.f
	);

	ATile* NewTile = GetWorld()->SpawnActor<ATile>(
		TileClass,
		SpawnLocation,
		FRotator::ZeroRotator
	);

	if (!NewTile)
	{
		return nullptr;
	}

	// Setto la pisizione nella griglia
	NewTile->SetGridPosition(X, Y);

	// Setto l'altezza della tile
	NewTile->SetHeightLevel(HeightLevel);

	TileArray.Add(NewTile);
	TileMap.Add(FVector2D(X, Y), NewTile);

	return NewTile;
}

// Getter della tile nella posizione X,Y
ATile* AGameField::GetTileAtPosition(int32 X, int32 Y) const
{
	const FVector2D Key(X, Y);

	if (TileMap.Contains(Key))
	{
		return TileMap[Key];
	}

	return nullptr;
}