// Fill out your copyright notice in the Description page of Project Settings.


#include "HumanPlayer.h"
#include "GameField.h"
#include "TBS_GameMode.h"
#include "EngineUtils.h"

// Sets default values
AHumanPlayer::AHumanPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	// create a camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	//set the camera as RootComponent
	SetRootComponent(Camera);
    // get the game instance reference
    GameInstance = Cast<UTBS_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	// Inizializzo valori default
	PlayerID = 0;
    ClassToSpawn = nullptr;
    IsMyTurn = false;
    bHasPlacedSniper = false;
    bHasPlacedBrawler = false;
    bIsShowingMovementRange = false;
    bFirstUnitHasActed = false;
}

// Called when the game starts or when spawned
void AHumanPlayer::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AHumanPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AHumanPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind clic sinistro
    PlayerInputComponent->BindAction("LeftMouseClick", IE_Pressed, this, &AHumanPlayer::OnClick);
}

void AHumanPlayer::OnPlacementTurnStart()
{
    UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Il tuo turno di piazzamento"));
    IsMyTurn = true;

    if (!SniperClass || !BrawlerClass)
    {
        UE_LOG(LogTemp, Error, TEXT("HumanPlayer: SniperClass o BrawlerClass non assegnate"));
        GameInstance->SetTurnMessage(TEXT("Errore: Classi unità non assegnate"));
        return;
    }

    // Se entrambe le unità non sono state piazzate è il primo turno e si può scegliere che unità piazzare
    if (!bHasPlacedSniper && !bHasPlacedBrawler)
    {
        // Primo turno: scegliere tra sniper e brawler
        GameInstance->SetTurnMessage(TEXT("Scegli quale unita' piazzare! Premi 1 per Sniper o 2 per Brawler"));
        UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Scegli tra Sniper e Brawler"));
        ClassToSpawn = nullptr;
    }
    else if (!bHasPlacedSniper)
    {
        // Se bisogna piazzare sniper
        ClassToSpawn = SniperClass;
        PendingUnitTypeToSpawn = EUnitType::SNIPER;
        GameInstance->SetTurnMessage(TEXT("Piazza lo Sniper! Clicca su una tile in Y=0,1,2"));
        UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Piazza Sniper"));
    }
    else if (!bHasPlacedBrawler)
    {
        // Se bisogna piazzare brawler
        ClassToSpawn = BrawlerClass;
        PendingUnitTypeToSpawn = EUnitType::BRAWLER;
        GameInstance->SetTurnMessage(TEXT("Piazza il Brawler! Clicca su una tile in Y=0,1,2"));
        UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Piazza Brawler"));
    }
    else
    {
        GameInstance->SetTurnMessage(TEXT("Piazzamento iniziale completato!"));
        UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Piazzamento completato"));
    }
}

void AHumanPlayer::OnTurnStart()
{
	IsMyTurn = true;
	UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Il tuo turno"));
    // Resetto variabile per vedere se la prima unità ha effettuato azioni
    bFirstUnitHasActed = false;

    // Reset delle unità e del turno
    for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
    {
        AUnit* Unit = *It;
        if (Unit && Unit->OwnerPlayerID == PlayerID && Unit->IsAlive())
        {
            Unit->ResetTurnStatus();
        }
    }
    GameInstance->SetTurnMessage(TEXT("E' il tuo turno, muovi le tue unita'"));
}

void AHumanPlayer::OnTurnEnd()
{
	IsMyTurn = false;
	UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Turno terminato"));
}

bool AHumanPlayer::PendingTurnActions() const
{
    // Se non è il mio turno, nessuna azione disponibile
    if (!IsMyTurn)
    {
        return false;
    }

    // Fase piazzamento
    if (ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode()))
    {
        if (GM->bPlacementPhase)
        {
            return !(bHasPlacedSniper && bHasPlacedBrawler);
        }
    }

    // Controllo se le unità hanno sia mosso che attaccato (allora fine turno automatica, return false)
    int32 UnitsCompleted = 0;
    int32 TotalUnits = 0;

    for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
    {
        AUnit* Unit = *It;
        if (Unit && Unit->OwnerPlayerID == PlayerID && Unit->IsAlive())
        {
            TotalUnits++;

            // Se ha mosso e attaccato allora aggiungo l'unità
            if (Unit->bHasMovedThisTurn && Unit->bHasAttackedThisTurn)
            {
                UnitsCompleted++;
            }
        }
    }

    // Se tutte hanno completato return false, altrimenti ci sono ancora azioni disponibili da fare quindi ritorno true
    return (UnitsCompleted < TotalUnits);
}

void AHumanPlayer::SelectUnit(AUnit* Unit)
{
    if (!Unit)
    {
        UE_LOG(LogTemp, Warning, TEXT("HumanPlayer: Unita' nulla"));
        return;
    }

    ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField) return;

    // Se bFirstUnitHasActed è true allora posso selezionare solo l'altra unità che deve ancora muovere/attaccare
    if (bFirstUnitHasActed)
    {
        // Se provo a selezionare un'unità che ha già mosso o attaccato allora messaggio
        if (Unit->bHasMovedThisTurn || Unit->bHasAttackedThisTurn)
        {
            if (GameInstance)
            {
                GameInstance->SetTurnMessage(TEXT("Non puoi selezionare un'unita' che ha gia' mosso o attaccato!"));
            }
            return;
        }
    }

    // Se clicco sulla stessa unità devo nascondere il range di movimento se è già mostrato, altrimento lo mostro
    if (SelectedUnit == Unit)
    {
        
        if (bIsShowingMovementRange)
        {
            // Nascondo range
            HideMovementRange();
            UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Range nascosto"));
            if (GameInstance)
            {
                GameInstance->SetTurnMessage(TEXT("Range nascosto - Clicca di nuovo per mostrarlo"));
            }
            FVector2D UnitPos = Unit->GetCurrentGridPosition();
            ATile* CurrentTile = GM->GField->GetTileAtPosition(FMath::RoundToInt(UnitPos.X), FMath::RoundToInt(UnitPos.Y));
            if (CurrentTile)
            {
                CurrentTile->HighlightTile(false);
            }

            // Deseleziona completamente
            SelectedUnit = nullptr;
        }
        else
        {
            // Mostro range
            ShowMovementRange(Unit);
            if (GameInstance)
            {
                GameInstance->SetTurnMessage(TEXT("Range mostrato"));
            }
        }
        return;
    }

    // Altrimenti ho selezionato una nuova unità

    // Nascondo range movimento precedente
    HideMovementRange();

    // Deseleziono l'unità
    if (SelectedUnit)
    {
        FVector2D PrevPos = SelectedUnit->GetCurrentGridPosition();
        ATile* PrevTile = GM->GField->GetTileAtPosition(FMath::RoundToInt(PrevPos.X), FMath::RoundToInt(PrevPos.Y));
        if (PrevTile)
        {
            PrevTile->HighlightTile(false);
        }
        UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Deselezionata unita' precedente"));
    }

    // Seleziono la nuova unità
    SelectedUnit = Unit;

    FVector2D UnitPos = Unit->GetCurrentGridPosition();
    ATile* CurrentTile = GM->GField->GetTileAtPosition(FMath::RoundToInt(UnitPos.X), FMath::RoundToInt(UnitPos.Y));

    if (CurrentTile)
    {
        CurrentTile->HighlightTile(true);
        UE_LOG(LogTemp, Log, TEXT("Tile evidenziata sotto l'unita'"));
    }

    // Mostro il range di movimento quando seleziono una nuova unità
    ShowMovementRange(Unit);

    UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Unita' selezionata - %s"), *Unit->GetName());

    // Messaggi GameInstance
    if (GameInstance)
    {
        FString StatusMsg;
        if (Unit->bHasMovedThisTurn && Unit->bHasAttackedThisTurn)
        {
            StatusMsg = TEXT("(ha gia' mosso e attaccato)");
        }
        else if (Unit->bHasMovedThisTurn)
        {
            StatusMsg = TEXT("(ha gia' mosso, puo' attaccare)");
        }
        else if (Unit->bHasAttackedThisTurn)
        {
            StatusMsg = TEXT("(ha gia' attaccato)");
        }
        else
        {
            StatusMsg = TEXT("(pronta)");
        }

        GameInstance->SetTurnMessage(FString::Printf(TEXT("Unita': %s %s"), *Unit->GetName(), *StatusMsg));
    }
}

void AHumanPlayer::OnWin()
{
	GameInstance->SetTurnMessage("Hai vinto la partita!");
}

void AHumanPlayer::OnLose()
{
	UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Hai perso la partita"));
}

void AHumanPlayer::OnClick()
{
    UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Clic sinistro rilevato"));

    // Raycast sotto il cursore
    FHitResult Hit;
    bool bHit = GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, Hit);

    if (!bHit || !Hit.GetActor())
    {
        UE_LOG(LogTemp, Log, TEXT("Clic su niente o non valido"));
        return;
    }

    AActor* HitActor = Hit.GetActor();
    UE_LOG(LogTemp, Log, TEXT("Cliccato su: %s"), *HitActor->GetName());

    // Prendo gamemode
    ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode());
    if (!GM)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameMode non trovato!"));
        return;
    }

    // Caso 1: click su una tile
	if (ATile* HitTile = Cast<ATile>(HitActor))
	{
		FVector2D TilePos = HitTile->GetGridPosition();
		int32 TileX = FMath::RoundToInt(TilePos.X);
		int32 TileY = FMath::RoundToInt(TilePos.Y);
		UE_LOG(LogTemp, Log, TEXT("Tile cliccata: (%d, %d)"), TileX, TileY);

        // Fase piazzamento iniziale
        if (IsMyTurn && GM->bPlacementPhase)
        {
            // Controllo se ClassToSpawn è null
            if (!ClassToSpawn)
            {
                UE_LOG(LogTemp, Warning, TEXT("Devi prima selezionare un'unita', premi 1 per Sniper o 2 per Brawler"));
                GameInstance->SetTurnMessage(TEXT("Premi 1 per Sniper o 2 per Brawler"));
                return;
            }
            // Verifico se sono nella zona di piazzamento e se la tile è camminabile
            if (TileY >= 0 && TileY <= 2 && HitTile->IsWalkable())
            {
                UE_LOG(LogTemp, Log, TEXT("Tile valida per piazzamento"));

                FVector SpawnLoc = HitTile->GetActorLocation();
                SpawnLoc.Z += 150.f;
                FRotator SpawnRotation = FRotator(0.f, 90.f, 0.f);

                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                AUnit* NewUnit = GetWorld()->SpawnActor<AUnit>(ClassToSpawn, SpawnLoc, SpawnRotation, SpawnParams);
                if (NewUnit)
                {
                    NewUnit->OwnerPlayerID = 0;
                    NewUnit->SetCurrentGridPosition(TilePos);
                    NewUnit->InitialGridPosition = TilePos;

                    // Aggiorno quale unità deve essere spawnata
                    if (PendingUnitTypeToSpawn == EUnitType::SNIPER)
                    {
                        bHasPlacedSniper = true;
                        UE_LOG(LogTemp, Log, TEXT("Sniper piazzato con successo"));
                        GameInstance->SetTurnMessage(TEXT("Sniper piazzato"));
                    }
                    else if (PendingUnitTypeToSpawn == EUnitType::BRAWLER)
                    {
                        bHasPlacedBrawler = true;
                        UE_LOG(LogTemp, Log, TEXT("Brawler piazzato con successo"));
                        GameInstance->SetTurnMessage(TEXT("Brawler piazzato"));
                    }

                    UE_LOG(LogTemp, Log, TEXT("%s piazzato in (%d, %d)"), *NewUnit->GetName(), TileX, TileY);

                    // Reset ClassToSpawn
                    ClassToSpawn = nullptr;
                    // Mando notifica a gamemode
                    GM->OnUnitPlaced(0);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Tile non valida per piazzamento (fuori da Y=0,1,2 o non camminabile)"));
                GameInstance->SetTurnMessage(TEXT("Tile non valida (Y=0-2 e camminabile)"));
            }
        }
        // Fase movimento
        else if (SelectedUnit)
        {
            UE_LOG(LogTemp, Log, TEXT("Clic su tile - movimento"));

            // Se l'unità ha attaccato non può muoversi
            if (SelectedUnit->bHasAttackedThisTurn)
            {
                UE_LOG(LogTemp, Warning, TEXT("L'unita' ha gia' attaccato, non puo' muoversi"));
                GameInstance->SetTurnMessage(TEXT("L'unita' ha gia' attaccato, non puo' muoversi"));
                return;
            }

            // Controllo se l'unità è stata mossa nel turno
            if (SelectedUnit->bHasMovedThisTurn)
            {
                UE_LOG(LogTemp, Warning, TEXT("Questa unita' ha gia' mosso"));
                GameInstance->SetTurnMessage(TEXT("Unita' gia' mossa, puo' solo attaccare oppure seleziona un'altra unita'"));
                return;
            }

            // Muovo l'unità selezionata se possibile
            if (SelectedUnit->CanMoveTo(TileX, TileY, GM->GField))
            {
                FVector2D OldPos = SelectedUnit->GetCurrentGridPosition();
                // Nascondo range di movimento
                HideMovementRange();
                // Rimuovo highlight tile
                ATile* OldTile = GM->GField->GetTileAtPosition(FMath::RoundToInt(OldPos.X), FMath::RoundToInt(OldPos.Y));
                
                SelectedUnit->MoveTo(TileX, TileY, GM->GField);
                UE_LOG(LogTemp, Log, TEXT("Unita' mossa"));
                GameInstance->SetTurnMessage(TEXT("Unita' mossa"));

                if (OldTile)
                {
                    OldTile->HighlightTile(false);
                }

                // Evidenzio la nuova tile
                ATile* NewTile = GM->GField->GetTileAtPosition(TileX, TileY);
                if (NewTile)
                {
                    NewTile->HighlightTile(true);
                }
                // Blocco unità
                bFirstUnitHasActed = true;
                int32 UnitsActed = 0;
                for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
                {
                    AUnit* U = *It;
                    if (U && U->OwnerPlayerID == PlayerID && U->IsAlive())
                    {
                        if (U->bHasMovedThisTurn || U->bHasAttackedThisTurn)
                        {
                            UnitsActed++;
                        }
                    }
                }

                // Messaggio diverso se è la prima o seconda unità
                if (UnitsActed == 1)
                {
                    GameInstance->SetTurnMessage(TEXT("Unita' mossa - Attacca o seleziona la seconda unita'"));
                }
                else if (UnitsActed == 2)
                {
                    GameInstance->SetTurnMessage(TEXT("Unita' mossa - Attacca o usa il bottone per terminare"));
                }

                // Controllo fine turno
                CheckAndEndTurnIfComplete();
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Movimento non riuscito"));
                GameInstance->SetTurnMessage(TEXT("Movimento non riuscito"));
            }
        }
    }
    // Caso 2: clic su una unità
    else if (AUnit* HitUnit = Cast<AUnit>(HitActor))
    {
        // Selezione unità mia
        if (HitUnit->OwnerPlayerID == 0 && IsMyTurn && !GM->bPlacementPhase)
        {
            SelectUnit(HitUnit);
            UE_LOG(LogTemp, Log, TEXT("Unità selezionata"));
            GameInstance->SetTurnMessage(TEXT("Unita' selezionata"));
        }
        // Attacco unità nemica (se selezionata unità mia)
        else if (SelectedUnit && HitUnit->OwnerPlayerID != 0 && IsMyTurn)
        {
            // Controllo se l'unità ha già attaccato nel turno
            if (SelectedUnit->bHasAttackedThisTurn)
            {
                UE_LOG(LogTemp, Warning, TEXT("Questa unita' ha gia' attaccato"));
                GameInstance->SetTurnMessage(TEXT("L'unita' ha gia' attaccato, seleziona un'altra unita'"));
                return;
            }
            if (SelectedUnit->CanAttack(HitUnit, GM->GField))
            {
                int32 Damage = SelectedUnit->CalculateDamage();
                HitUnit->ApplyDamage(Damage);
                UE_LOG(LogTemp, Log, TEXT("Attacco riuscito"));
                GameInstance->SetTurnMessage(FString::Printf(TEXT("Attacco riuscito! Danno: %d"), Damage));

                // Nascondo range movimento
                HideMovementRange();

                FVector2D AttackerPos = SelectedUnit->GetCurrentGridPosition();
                ATile* AttackerTile = GM->GField->GetTileAtPosition(FMath::RoundToInt(AttackerPos.X), FMath::RoundToInt(AttackerPos.Y));
                if (AttackerTile)
                {
                    AttackerTile->HighlightTile(false);
                }

                // Blocco unità
                bFirstUnitHasActed = true;
                UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Prima unita' ha attaccato ora bloccata"));

                SelectedUnit = nullptr;

                GameInstance->SetTurnMessage(FString::Printf(TEXT("Attacco riuscito! Danno: %d"), Damage));

                CheckAndEndTurnIfComplete();
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Attacco non possibile"));
                GameInstance->SetTurnMessage(TEXT("Attacco non possibile"));
            }
        }
    }
}

void AHumanPlayer::SelectSniperForPlacement()
{
    ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->bPlacementPhase || !IsMyTurn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Non puoi selezionare ora"));
        return;
    }

    // HumanPlayer può selezionare solo se è il primo turno ed entrambe le unità non sono ancora state piazzate
    if (bHasPlacedSniper || bHasPlacedBrawler)
    {
        UE_LOG(LogTemp, Warning, TEXT("Non puoi piu' scegliere, devi piazzare la seconda unita'"));
        GameInstance->SetTurnMessage(TEXT("Devi piazzare l'unita' rimanente"));
        return;
    }

    // Selezionato Sniper
    ClassToSpawn = SniperClass;
    PendingUnitTypeToSpawn = EUnitType::SNIPER;
    UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Sniper selezionato"));
    GameInstance->SetTurnMessage(TEXT("Sniper selezionato, clicca su tile (Y=0,1,2)"));
}

void AHumanPlayer::SelectBrawlerForPlacement()
{
    ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->bPlacementPhase || !IsMyTurn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Non puoi selezionare ora"));
        return;
    }

    // HumanPlayer può selezionare solo se è il primo turno ed entrambe le unità non sono ancora state piazzate
    if (bHasPlacedSniper || bHasPlacedBrawler)
    {
        UE_LOG(LogTemp, Warning, TEXT("Non puoi piu' scegliere, devi piazzare la seconda unita'"));
        GameInstance->SetTurnMessage(TEXT("Devi piazzare l'unita' rimanente"));
        return;
    }

    // Selezionato brawler
    ClassToSpawn = BrawlerClass;
    PendingUnitTypeToSpawn = EUnitType::BRAWLER;
    UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Brawler selezionato"));
    GameInstance->SetTurnMessage(TEXT("Brawler selezionato, clicca su tile (Y=0-2)"));
}

void AHumanPlayer::CheckAndEndTurnIfComplete()
{
    ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || GM->bPlacementPhase) return;

    // Chiamo PendingTurnActions() per controllare se il turno è finito
    if (!PendingTurnActions())
    {
        UE_LOG(LogTemp, Warning, TEXT("HumanPlayer: Tutte le unità completate - Fine turno automatica"));

        if (GameInstance)
        {
            GameInstance->SetTurnMessage(TEXT("Turno completato"));
        }

        HideMovementRange();

        if (SelectedUnit)
        {
            FVector2D UnitPos = SelectedUnit->GetCurrentGridPosition();
            ATile* UnitTile = GM->GField->GetTileAtPosition(
                FMath::RoundToInt(UnitPos.X),
                FMath::RoundToInt(UnitPos.Y)
            );
            if (UnitTile)
            {
                UnitTile->HighlightTile(false);
            }
        }

        SelectedUnit = nullptr;
        bFirstUnitHasActed = false;

        // Fine turno
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, GM]()
            {
                GM->TurnNextPlayer(PlayerID);
            }, 1.5f, false);
    }
}

void AHumanPlayer::ShowMovementRange(AUnit* Unit)
{
    if (!Unit) return;

    ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GField) return;

    // Nascondo range precedente se ce ne fosse ancora uno per evitare bug
    HideMovementRange();

    UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Range movimento per %s mostrato"), *Unit->GetName());

    // Chiamo GetReachableTiles per ottenere le tile raggiungibili
    TArray<FIntPoint> ReachableTiles = Unit->GetReachableTiles(GM->GField);

    // Per le tile raggiungibili mostro un overlay bianco
    for (const FIntPoint& TilePos : ReachableTiles)
    {
        ATile* Tile = GM->GField->GetTileAtPosition(TilePos.X, TilePos.Y);
        if (Tile)
        {
            Tile->ShowMovementOverlay(true);
            HighlightedMovementTiles.Add(Tile);
        }
    }

    bIsShowingMovementRange = true;
    UE_LOG(LogTemp, Log, TEXT("HumanPlayer: %d tile evidenziate"), HighlightedMovementTiles.Num());
}

void AHumanPlayer::HideMovementRange()
{
    if (HighlightedMovementTiles.Num() == 0) return;

    UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Nascondo range movimento"));

    // A tutte le tile evidenziate rimuovo l'overlay
    for (ATile* Tile : HighlightedMovementTiles)
    {
        if (Tile && IsValid(Tile))
        {
            Tile->ShowMovementOverlay(false);
        }
    }

    HighlightedMovementTiles.Empty();
    bIsShowingMovementRange = false;
}

void AHumanPlayer::EndTurnWithoutAttack()
{
    ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || GM->bPlacementPhase || !IsMyTurn) return;

    UE_LOG(LogTemp, Warning, TEXT("HumanPlayer: Turno terminato senza attaccare (bottone)"));

    HideMovementRange();

    if (SelectedUnit)
    {
        FVector2D UnitPos = SelectedUnit->GetCurrentGridPosition();
        ATile* UnitTile = GM->GField->GetTileAtPosition(FMath::RoundToInt(UnitPos.X), FMath::RoundToInt(UnitPos.Y));
        if (UnitTile)
        {
            UnitTile->HighlightTile(false);
        }
    }

    SelectedUnit = nullptr;
    bFirstUnitHasActed = false;

    if (GameInstance)
    {
        GameInstance->SetTurnMessage(TEXT("Turno terminato"));
    }

    // Passa al turno AI
    GM->TurnNextPlayer(PlayerID);
}

bool AHumanPlayer::ShowEndTurnButton() const
{
    if (!IsMyTurn) return false;

    ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode());
    if (!GM || GM->bPlacementPhase) return false;

    // Conta unità che hanno agito
    int32 UnitsActed = 0;
    int32 TotalUnits = 0;
    bool bAtLeastOneOnlyMoved = false;

    for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
    {
        AUnit* Unit = *It;
        if (Unit && Unit->OwnerPlayerID == PlayerID && Unit->IsAlive())
        {
            TotalUnits++;

            if (Unit->bHasMovedThisTurn || Unit->bHasAttackedThisTurn)
            {
                UnitsActed++;
            }

            // Almeno una ha solo mosso (senza attaccare)
            if (Unit->bHasMovedThisTurn && !Unit->bHasAttackedThisTurn)
            {
                bAtLeastOneOnlyMoved = true;
            }
        }
    }

    // Mostro il bottone se:
    // -Entrambe le unità hanno agito
    // -Almeno una ha solo mosso
    // -Ci sono ancora azioni disponibili da fare
    bool bShouldShow = (UnitsActed == TotalUnits && bAtLeastOneOnlyMoved && PendingTurnActions());

    UE_LOG(LogTemp, Log, TEXT("ShowEndTurnButton: UnitsActed=%d, TotalUnits=%d, OnlyMoved=%s, Pending=%s -> %s"),
        UnitsActed, TotalUnits,
        bAtLeastOneOnlyMoved ? TEXT("SI") : TEXT("NO"),
        PendingTurnActions() ? TEXT("SI") : TEXT("NO"),
        bShouldShow ? TEXT("MOSTRA") : TEXT("NASCONDI")
    );

    return bShouldShow;
}
