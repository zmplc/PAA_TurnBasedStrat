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
};