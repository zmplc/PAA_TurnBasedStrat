// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

// Status of the tile: empty or occupied
UENUM()
enum class ETileStatus : uint8
{
	EMPTY     UMETA(DisplayName = "Empty"),
	OCCUPIED  UMETA(DisplayName = "Occupied"),
};

// Tipo della tile: water, ground, hill, mountain, high mountain, tower
UENUM(BlueprintType)
enum class ETileType : uint8
{
	WATER			UMETA(DisplayName = "Water"),
	GROUND			UMETA(DisplayName = "Ground"),
	HILL			UMETA(DisplayName = "Hill"),
	MOUNTAIN		UMETA(DisplayName = "Mountain"),
	HIGHMOUNTAIN	UMETA(DisplayName = "High Mountain"),
	TOWER			UMETA(DisplayName = "Tower")
};

UCLASS()
class PAA_TURNBASEDSTRAT_API ATile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATile();

	// set the player owner and the status of a tile
	void SetTileStatus(const int32 TileOwner, const ETileStatus TileStatus);

	// get the tile status
	ETileStatus GetTileStatus();

	// get the tile owner
	int32 GetOwner();

	// set the (x, y) position
	void SetGridPosition(const double InX, const double InY);

	// get the (x, y) position
	FVector2D GetGridPosition();

	// Setter e getter per altezza della tile
	void SetHeightLevel(int32 InHeightLevel);
	int32 GetHeightLevel() const;

	// Setter e getter per tipo della tile
	void SetTileType(ETileType InTileType);
	ETileType GetTileType() const;

	// Funzione per fare il check se tile è walkable
	bool IsWalkable() const;

	// Funzione update colore tile
	void UpdateTileColor();

	// Funzione update altezza tile
	void UpdateTileHeight();

	// Funzione per fare l'highlight della tile selezionata
	void HighlightTile(bool bHighlight);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ETileStatus Status;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PlayerOwner;

	// (x, y) position of the tile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D TileGridPosition;

	// Livello della tile (da 0 a 4)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	int32 HeightLevel;

	// Tipo tile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	ETileType TileType;

	// Proprietà walkable
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	bool bWalkable;

	// MaterialIstanceDynamic per cambiare colore della tile
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	// Colore originale tile pre-highlight
	UPROPERTY()
	FLinearColor OriginalColor;

	// Se la tile è highlighted o no
	UPROPERTY()
	bool bIsHighlighted;
};