// Fill out your copyright notice in the Description page of Project Settings.


#include "HumanPlayer.h"
#include "GameField.h"
#include "TBS_GameMode.h"

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
        GameInstance->SetTurnMessage(TEXT("Scegli quale unità piazzare! Premi 1 per Sniper o 2 per Brawler"));
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
}

void AHumanPlayer::OnTurnEnd()
{
	IsMyTurn = false;
	UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Turno terminato"));
}

bool AHumanPlayer::PendingTurnActions() const
{
    // Se non è il mio turno non ho azioni da compiere allora return false
    if (!IsMyTurn)
    {
        return false;
    }

    // Durante fase di piazzamento
    if (ATBS_GameMode* GM = Cast<ATBS_GameMode>(GetWorld()->GetAuthGameMode()))
    {
        if (GM->bPlacementPhase)
        {
            // Se non ho piazzato ancora unità nella fase iniziale allora ho azioni rimanenti
            return !(bHasPlacedSniper && bHasPlacedBrawler);
        }
    }

    // Durante il gioco
    // TODO: controllo se ci sono unità che non hanno ancora mosso/attaccato
    return true;
}

void AHumanPlayer::SelectUnit(AUnit* Unit)
{
    if (!Unit)
    {
        UE_LOG(LogTemp, Warning, TEXT("HumanPlayer: Unita' nulla"));
        return;
    }

    // Deseleziona unità precedente se presente
    if (SelectedUnit)
    {
        // TODO: se deseleziono una unità allora tolgo highlight range movimento
        UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Deselezionata unita'"));
    }

    // Seleziono una nuova unità
    SelectedUnit = Unit;
    UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Unita' selezionata - %s"), *Unit->GetName());

    if (GameInstance)
    {
        GameInstance->SetTurnMessage(FString::Printf(TEXT("Unita' selezionata: %s"), *Unit->GetName()));
    }

    // TODO: aggiungere highlight per mostrare range movimento
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
                UE_LOG(LogTemp, Warning, TEXT("Devi prima selezionare un'unità! Premi 1 per Sniper o 2 per Brawler"));
                GameInstance->SetTurnMessage(TEXT("Premi 1 per Sniper o 2 per Brawler"));
                return;
            }

            if (TileY >= 0 && TileY <= 2 && HitTile->IsWalkable())
            {
                UE_LOG(LogTemp, Log, TEXT("Tile valida per piazzamento"));

                FVector SpawnLoc = HitTile->GetActorLocation();
                SpawnLoc.Z += 150.f;
                FRotator SpawnRotation = FRotator(0.f, 90.f, 0.f);

                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

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
                        GameInstance->SetTurnMessage(TEXT("Sniper piazzato!"));
                    }
                    else if (PendingUnitTypeToSpawn == EUnitType::BRAWLER)
                    {
                        bHasPlacedBrawler = true;
                        UE_LOG(LogTemp, Log, TEXT("Brawler piazzato con successo"));
                        GameInstance->SetTurnMessage(TEXT("Brawler piazzato!"));
                    }

                    UE_LOG(LogTemp, Log, TEXT("%s piazzato in (%d, %d)"), *NewUnit->GetName(), TileX, TileY);

                    // Reset ClassToSpawn
                    ClassToSpawn = nullptr;

                    GM->OnUnitPlaced(0);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Tile non valida per piazzamento (fuori da Y=0,1,2 o non camminabile)"));
                GameInstance->SetTurnMessage(TEXT("Tile non valida! (Y=0-2 e camminabile)"));
            }
        }
        // Fase movimento
        else if (SelectedUnit)
        {
            UE_LOG(LogTemp, Log, TEXT("Clic su tile - movimento"));

            // Muovo l'unità selezionata se possibile
            if (SelectedUnit->CanMoveTo(TileX, TileY, GM->GField))
            {
                SelectedUnit->MoveTo(TileX, TileY, GM->GField);
                UE_LOG(LogTemp, Log, TEXT("Unità mossa"));
                GameInstance->SetTurnMessage(TEXT("Unità mossa"));
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
            GameInstance->SetTurnMessage(TEXT("Unità selezionata"));
        }
        // Attacco unità nemica (se selezionata unità mia)
        else if (SelectedUnit && HitUnit->OwnerPlayerID != 0 && IsMyTurn)
        {
            if (SelectedUnit->CanAttack(HitUnit, GM->GField))
            {
                int32 Damage = SelectedUnit->CalculateDamage();
                HitUnit->ApplyDamage(Damage);
                UE_LOG(LogTemp, Log, TEXT("Attacco riuscito"));
                GameInstance->SetTurnMessage(FString::Printf(TEXT("Attacco! Danno: %d"), Damage));

                // Devo deselezionare l'unità dopo l'attacco perché per specifiche se attacco non posso effettuare movimento
                SelectedUnit = nullptr;
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
    GameInstance->SetTurnMessage(TEXT("Sniper selezionato - Clicca su tile (Y=0,1,2)"));
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
    GameInstance->SetTurnMessage(TEXT("Brawler selezionato - Clicca su tile (Y=0-2)"));
}
