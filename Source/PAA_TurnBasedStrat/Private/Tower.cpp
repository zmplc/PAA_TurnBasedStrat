// Fill out your copyright notice in the Description page of Project Settings.


#include "Tower.h"
#include "Unit.h"
#include "EngineUtils.h"

// Sets default values
ATower::ATower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// template function that creates a components
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));

	// every actor has a RootComponent that defines the transform in the World
	SetRootComponent(Scene);
	TowerMesh->SetupAttachment(Scene);
}

// Called when the game starts or when spawned
void ATower::BeginPlay()
{
	Super::BeginPlay();

	if (TowerMesh)
	{
		UMaterialInterface* BaseMaterial = TowerMesh->GetMaterial(0);
		if (BaseMaterial)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			TowerMesh->SetMaterial(0, DynamicMaterial);

			// Imposto stato iniziale della torre
			UpdateTowerStatus(ETowerStatus::NEUTRAL);
		}
	}
	
}

// Funzione per fare l'update dello stato della torre
void ATower::UpdateTowerStatus(ETowerStatus NewStatus, int32 NewControllingPlayerID)
{
	CurrentStatus = NewStatus;
	ControllingPlayerID = NewControllingPlayerID;

	if (DynamicMaterial)
	{
		FLinearColor TargetColor = FLinearColor::White;
		switch (CurrentStatus)
		{
		// Stato A: neutrale
		case ETowerStatus::NEUTRAL:
			TargetColor = NeutralColor;
			break;
		// Stato B: sotto controllo
		case ETowerStatus::CONTROLLED:
			// Controllato dal player umano
			if (ControllingPlayerID == 0)
			{
				TargetColor = PlayerHumanColor;
			}
			// Controllato da player AI
			else if (ControllingPlayerID == 1)
			{
				TargetColor = PlayerAIColor;
			}
			break;
		// Stato C: contesa
		case ETowerStatus::CONTESTED:
			TargetColor = ContestedColor;
			break;
		default:
			break;
		}
		// Aggiorno il colore della torre con il TargetColor
		DynamicMaterial->SetVectorParameterValue(FName("TowerColor"), TargetColor);
	}
	else {
		return;
	}
}

void ATower::CheckCaptureZone(AGameField* GameField)
{
	if (!GameField) return;

	// Chiamo GetUnitsInCaptureZone per ottenere tutte le unità che sono nella zona di cattura
	TArray<AUnit*> UnitsInCaptureZone = GetUnitsInCaptureZone(GameField);

	if (UnitsInCaptureZone.Num() == 0)
	{
		// Se non c'è nessuno nella zona di cattura la torre rimane neutrale o sotto controllo del giocatore che la ha conquistata
		return;
	}

	// Altrimenti ci sono delle unità e devo contare quali sono quelle di HumanPlayer e quali sono quelle dell'AI
	int32 HumanUnits = 0;
	int32 AiUnits = 0;
	for (AUnit* Unit : UnitsInCaptureZone)
	{
		if (Unit->OwnerPlayerID == 0)
		{
			HumanUnits++;
		}
		else if (Unit->OwnerPlayerID == 1)
		{
			AiUnits++;
		}
	}

	if (HumanUnits > 0 && AiUnits > 0)
	{
		// STATO C: CONTESA
		// Se ci sono unità di entrambi i giocatori nella zona di cattura allora la torre è contesa
		if (CurrentStatus != ETowerStatus::CONTESTED)
		{
			UE_LOG(LogTemp, Log, TEXT("Tower (%d,%d): CONTESA"),
				FMath::RoundToInt(GridPosition.X),
				FMath::RoundToInt(GridPosition.Y));;

			UpdateTowerStatus(ETowerStatus::CONTESTED, -1);
			ControllingPlayerID = -1;
		}
		return;
	}

	// STATO B: SOTTO CONTROLLO
	// Se ci sono unità solo di HumanPlayer o solo dell'AI nella zona di cattura allora la torre è sotto controllo del giocatore
	if (HumanUnits > 0)
	{
		if (CurrentStatus != ETowerStatus::CONTROLLED || ControllingPlayerID != 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Tower (%d,%d): CONTROLLATA da HUMAN"),
				FMath::RoundToInt(GridPosition.X),
				FMath::RoundToInt(GridPosition.Y));

			UpdateTowerStatus(ETowerStatus::CONTROLLED, 0);
			ControllingPlayerID = 0;
		}
	}
	else if (AiUnits > 0)
	{
		// AI controlla la torre
		if (CurrentStatus != ETowerStatus::CONTROLLED || ControllingPlayerID != 1)
		{
			UE_LOG(LogTemp, Log, TEXT("Tower (%d,%d): CONTROLLATA da AI"),
				FMath::RoundToInt(GridPosition.X),
				FMath::RoundToInt(GridPosition.Y));

			UpdateTowerStatus(ETowerStatus::CONTROLLED, 1);
			ControllingPlayerID = 1;
		}
	}
}

TArray<class AUnit*> ATower::GetUnitsInCaptureZone(AGameField* GameField)
{
	// Inizializzo array delle unità nella zona di cattura
	TArray<AUnit*> UnitsInCaptureZone;

	if (!GameField) return UnitsInCaptureZone;

	// Per ogni unità nella griglia
	for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
	{
		AUnit* Unit = *It;
		// Se l'unità non è valida o non è viva allora continue
		if (!Unit || !Unit->IsAlive()) continue;
		// Salvo posizione unità
		FVector2D UnitPos = Unit->GetCurrentGridPosition();

		// Da specifiche la zona di cattura è l'insieme di celle che hanno una distanza minore o uguale a 2 dalla coordinata di una torre anche in diagonale
		// Siccome devo includere le diagonali implemento la distanza Chebyshev: quindi come esempio avrei d(x,y)=max(|x1-x2|,|y1-y2|)
		int32 DistanceX = FMath::Abs(UnitPos.X - GridPosition.X);
		int32 DistanceY = FMath::Abs(UnitPos.Y - GridPosition.Y);
		int32 Distance = FMath::Max(DistanceX, DistanceY);

		// Se Distance è minore o uguale a 2 allora l'unità è nella zona di cattura e la aggiungo in UnitsInCaptureZone
		if (Distance <= 2) {
			UnitsInCaptureZone.Add(Unit);
			UE_LOG(LogTemp, Log, TEXT("Tower (%d,%d): Unità %s in zona cattura (distanza %d)"),
				FMath::RoundToInt(GridPosition.X),
				FMath::RoundToInt(GridPosition.Y),
				*Unit->GetName(),
				Distance);
		}
	}
	// Faccio return dell'array con le unità nella zona di cattura
	return UnitsInCaptureZone;
}

// Called every frame
void ATower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

