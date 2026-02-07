// Fill out your copyright notice in the Description page of Project Settings.


#include "HumanPlayer.h"
#include "GameField.h"

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
	PlayerID = -1;
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
    UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Il tuo turno di piazzamento. Scegli una tile in Y = 0,1,2"));
    IsMyTurn = true;

    // TODO
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

void AHumanPlayer::PerformTurnActions()
{
}

bool AHumanPlayer::PendingTurnActions() const
{
	return true;
}

void AHumanPlayer::SelectUnit(AUnit* Unit)
{

	UE_LOG(LogTemp, Log, TEXT("HumanPlayer: Unità selezionata"));
}

void AHumanPlayer::OnWin()
{
	if (UTBS_GameInstance* GI = Cast<UTBS_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		GI->SetTurnMessage("Hai vinto la partita!");
	}
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

        // Fase piazzamento iniziale se è il mio turno e bPlacementPhase è true
        if (IsMyTurn && GM->bPlacementPhase)
        {
            // Controllo se sono in zona piazzamento humanplayer (Y=0,1,2) e camminabile
            if (TileY >= 0 && TileY <= 2 && HitTile->IsWalkable())
            {
                UE_LOG(LogTemp, Log, TEXT("Tile valida per piazzamento"));

                // Spawn unità scelta (uso Sniper per prova
                if (UnitToSpawnClass)
                {
                    FVector SpawnLoc = HitTile->GetActorLocation();
                    SpawnLoc.Z += 50.f;

                    AUnit* NewUnit = GetWorld()->SpawnActor<AUnit>(UnitToSpawnClass, SpawnLoc, FRotator::ZeroRotator);
                    if (NewUnit)
                    {
                        NewUnit->OwnerPlayerID = 0;
                        NewUnit->SetCurrentGridPosition(TilePos);
                        UE_LOG(LogTemp, Log, TEXT("%s piazzato in (%d, %d)"), *NewUnit->GetName(), TileX, TileY);
                        GM->OnUnitPlaced(0);
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Nessuna unità selezionata per piazzamento"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Tile non valida per piazzamento (fuori da Y=0,1,2 o non camminabile)"));
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
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Movimento non riuscito"));
            }
        }
    }
    // Caso 2: clic su una unità
    else if (AUnit* HitUnit = Cast<AUnit>(HitActor))
    {
        // Selezione unità mia
        if (HitUnit->OwnerPlayerID == 0 && IsMyTurn)
        {
            SelectUnit(HitUnit);
            UE_LOG(LogTemp, Log, TEXT("Unità selezionata"));
        }
        // Attacco unità nemica (se selezionata unità mia)
        else if (SelectedUnit && HitUnit->OwnerPlayerID != 0 && IsMyTurn)
        {
            if (SelectedUnit->CanAttack(HitUnit, GM->GField))
            {
                int32 Damage = SelectedUnit->CalculateDamage();
                HitUnit->ApplyDamage(Damage);
                UE_LOG(LogTemp, Log, TEXT("Attacco riuscito"));
                // Devo deselezionare l'unità dopo l'attacco perché per specifiche se attacco non posso effettuare movimento
                SelectedUnit = nullptr;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Attacco non riuscito"));
            }
        }
    }
}
