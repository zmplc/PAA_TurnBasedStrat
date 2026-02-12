// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomPlayer.h"
#include "GameField.h"
#include "TBS_GameMode.h"
#include "Unit.h"
#include "Tile.h"

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
	UE_LOG(LogTemp, Log, TEXT("RandomPlayer: RandomPlayer creato"));

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
	UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Il mio turno"));

	if (GameInstance)
	{
		GameInstance->SetTurnMessage(TEXT("Turno AI"));
	}

	// TODO: da fare logica movimento e attacco
}

void ARandomPlayer::OnTurnEnd()
{
	IsMyTurn = false;
	UE_LOG(LogTemp, Log, TEXT("RandomPlayer: Turno terminato"));
}

void ARandomPlayer::PerformTurnActions()
{
}

bool ARandomPlayer::PendingTurnActions() const
{
	return false;
}

void ARandomPlayer::OnWin()
{
	GameInstance->SetTurnMessage(TEXT("L'AI ha vinto la partita!"));
	UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Hai vinto la partita!"));
}

void ARandomPlayer::OnLose()
{
	GameInstance->SetTurnMessage(TEXT("L'AI ha perso la partita!"));
	UE_LOG(LogTemp, Warning, TEXT("RandomPlayer: Hai perso la partita!"));
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
				// Se la tile è valida la aggiungo nell'array
				ValidTiles.Add(Tile);
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

	UE_LOG(LogTemp, Error, TEXT("RandomPélayer: Nessuna tile valida trovata nella zona Y=22,23,24"));
	return nullptr;
}
