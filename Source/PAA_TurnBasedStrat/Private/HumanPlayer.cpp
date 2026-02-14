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

    // Reset delle unità e del turno
    for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
    {
        AUnit* Unit = *It;
        if (Unit && Unit->OwnerPlayerID == PlayerID && Unit->IsAlive())
        {
            Unit->ResetTurnStatus();
        }
    }
    GameInstance->SetTurnMessage(TEXT("Il tuo turno, muovi le tue unita'"));
}

void AHumanPlayer::OnTurnEnd()
{
	IsMyTurn = false;
	UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Turno terminato"));
}

bool AHumanPlayer::PendingTurnActions() const
{
    // Se non è il mio turno non ho azioni da fare, return false
    if (!IsMyTurn)
    {
        return false;
    }

    // Se sono in fase di piazzamento controllo se ho piazzato tutte le unità
    if (ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode()))
    {
        if (GM->bPlacementPhase)
        {
            return !(bHasPlacedSniper && bHasPlacedBrawler);
        }
    }

    // TODO: sistemare turno senza attacco
    // Dopo fase di piazzamento controllo se le unità hanno mosso/attaccato
    for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
    {
        AUnit* Unit = *It;
        if (Unit && Unit->OwnerPlayerID == PlayerID && Unit->IsAlive())
        {
            // Se entrambi i bool sono false allora return true perché ho azioni disponibili da fare
            if (!Unit->bHasMovedThisTurn && !Unit->bHasAttackedThisTurn)
            {
                return true;
            }
        }
    }
    // Altrimenti false
    return false;
}

void AHumanPlayer::SelectUnit(AUnit* Unit)
{
    if (ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode()))
    {
        if (!Unit)
        {
            UE_LOG(LogTemp, Warning, TEXT("HumanPlayer: Unità nulla"));
            return;
        }

        // Deseleziono unità precedente
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

        FVector2D UnitPos = SelectedUnit->GetCurrentGridPosition();
        ATile* CurrentTile = GM->GField->GetTileAtPosition(FMath::RoundToInt(UnitPos.X), FMath::RoundToInt(UnitPos.Y));

        if (CurrentTile)
        {
            CurrentTile->HighlightTile(true);
            UE_LOG(LogTemp, Log, TEXT("Tile evidenziata sotto l'unita'"));
        }
        UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Unità selezionata - %s"), *Unit->GetName());

        if (GameInstance)
        {
            FString StatusMsg;
            if (Unit->bHasMovedThisTurn && Unit->bHasAttackedThisTurn)
            {
                StatusMsg = TEXT("(ha gia' mosso e attaccato)");
            }
            else if (Unit->bHasMovedThisTurn)
            {
                StatusMsg = TEXT("(ha gia' mosso e puo' attaccare)");
            }
            else if (Unit->bHasAttackedThisTurn)
            {
                StatusMsg = TEXT("(ha gia' attaccato e puo' muovere)");
            }
            else
            {
                StatusMsg = TEXT("(pronta)");
            }

            GameInstance->SetTurnMessage(FString::Printf(TEXT("Unita': %s %s"), *Unit->GetName(), *StatusMsg));
        }
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
                ATile* OldTile = GM->GField->GetTileAtPosition(FMath::RoundToInt(OldPos.X), FMath::RoundToInt(OldPos.Y));
                
                SelectedUnit->MoveTo(TileX, TileY, GM->GField);
                UE_LOG(LogTemp, Log, TEXT("Unita' mossa"));
                GameInstance->SetTurnMessage(TEXT("Unita' mossa"));

                if (OldTile)
                {
                    OldTile->HighlightTile(false);
                }

                // Controlla fine turno
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
                GameInstance->SetTurnMessage(FString::Printf(TEXT("Attacco, danno: %d"), Damage));

                // Devo deselezionare l'unità dopo l'attacco perché per specifiche se attacco non posso effettuare movimento
                SelectedUnit = nullptr;
                // Controllo se ci sono azioni rimanenti
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
    if (!GM || GM->bPlacementPhase)
    {
        return;
    }

    // Creo un array con tutte le unità vive e uso TActorIterator per poi aggiungerle
    TArray<AUnit*> AliveUnits;
    for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
    {
        AUnit* Unit = *It;
        if (Unit && Unit->OwnerPlayerID == PlayerID && Unit->IsAlive())
        {
            AliveUnits.Add(Unit);
        }
    }
    // Se zero faccio return
    if (AliveUnits.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("HumanPlayer: Nessuna unità viva!"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Controllo unità - Totali: %d"), AliveUnits.Num());

    // Controllo se tutte le unità vive si sono mosse
    bool bAllUnitsActed = true;
    for (AUnit* Unit : AliveUnits)
    {
        // Una unità completa il suo turno se si è mossa, ha attaccato oppure entrambi
        bool bHasActed = Unit->bHasMovedThisTurn || Unit->bHasAttackedThisTurn;

        if (!bHasActed)
        {
            bAllUnitsActed = false;
            UE_LOG(LogTemp, Log, TEXT("  - %s: Non ha ancora effettuato azioni"), *Unit->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("  - %s: Ha agito (Mosso: %s, Attaccato: %s)"),
                *Unit->GetName(),
                Unit->bHasMovedThisTurn ? TEXT("SI") : TEXT("NO"),
                Unit->bHasAttackedThisTurn ? TEXT("SI") : TEXT("NO"));
        }
    }

    // Se le unità si sono mosse/hanno attaccato allora termino il turno
    if (bAllUnitsActed)
    {
        UE_LOG(LogTemp, Warning, TEXT("HumanPlayer: Tutte le unità hanno agito! Termino turno automaticamente"));

        if (GameInstance)
        {
            GameInstance->SetTurnMessage(TEXT("Tutte le unita' hanno agito - Turno AI..."));
        }

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

        // Deseleziono l'unità
        SelectedUnit = nullptr;

        // Delay di 1 secondo per passare al turno AI
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, GM]() {GM->TurnNextPlayer(PlayerID);}, 1.0f, false);
    }
}
