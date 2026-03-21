// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.h"
#include "Tower.h"
#include "ConfigData.h"
#include "GameField.generated.h"

UCLASS()
class PAA_TURNBASEDSTRAT_API AGameField : public AActor
{
	GENERATED_BODY()
	
public:
    // Sets default values for this actor's properties
    AGameField();

    // Called when an instance of this class is placed or spawned
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

    // Tile
    UPROPERTY(Transient)
    TArray<ATile*> TileArray;

    // Torri
    UPROPERTY()
    TArray<ATower*> TowerArray;

    UPROPERTY(Transient)
    TMap<FVector2D, ATile*> TileMap;

	// Classe della tile
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<ATile> TileClass;

    // Classe della tower
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<ATower> TowerClass;

	// Grid size (x= 25)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
    int32 GridSizeX = 25;

    // Grid size (y= 25)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
    int32 GridSizeY = 25;

	// Spazio tra le tile
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
    float TileSpacing = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
    float CellPadding = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    UConfigData* GridData;

	// Altezza massima delle tile (4)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
    int32 MaxHeight = 4;

	// Funzione per generare la griglia
    UFUNCTION(BlueprintCallable)
    void GenerateGrid();

	// Getter della posizione della tile
    UFUNCTION(BlueprintCallable)
    ATile* GetTileAtPosition(int32 X, int32 Y) const;

    // Getter delle torri presenti nella griglia
    UFUNCTION(BlueprintCallable)
    TArray<class ATower*> GetTowers();

protected:
	// Funzione per spawnare la tile
    ATile* SpawnTile(int32 X, int32 Y, int32 HeightLevel);

	// Funzione per posizionare le torri
    void PlaceTowers();
    FVector2D FindNearestWalkablePosition(const FVector2D& IdealPosition) const;

	// Funzione per verificare se tutte le celle sono raggiungibili
    bool IsMapFullyConnected() const;
    void FloodFill(int32 StartX, int32 StartY, TArray<bool>& Visited) const;
    ATile* FindFirstWalkableTile() const;

    // Funzione chiamata per far si che la zona di piazzamento sia connessa al resto della mappa (voglio evitare di avere solo oceano nelle zone di piazzamento)
    void MakeSpawnZonesConnected();

    // Costruisco un "ponte" di tile camminabili tra zona di piazzamento e la prima tile camminabile della mappa
    void CreateBridgeFromSpawnZone(int32 MinY, int32 MaxY, int32 SearchDirection);

    // Trova la tile camminabile più vicina alla zona di piazzamento, con una direzione specifica (verso basso per AI e verso alto per Human)
    FIntPoint NearestWalkableTileOutsideZone(int32 StartX, int32 StartY, int32 MinY, int32 MaxY) const;
};