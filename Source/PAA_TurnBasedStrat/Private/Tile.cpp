// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"

// Sets default values
ATile::ATile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// template function that creates a components
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	// every actor has a RootComponent that defines the transform in the World
	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

	Status = ETileStatus::EMPTY;
	PlayerOwner = -1;
	TileGridPosition = FVector2D(0, 0);

	// Valori default per la tile
	HeightLevel = 1;
	TileType = ETileType::GROUND;
	bWalkable = true;

}

void ATile::SetTileStatus(const int32 TileOwner, const ETileStatus TileStatus)
{
	PlayerOwner = TileOwner;
	Status = TileStatus;
}

ETileStatus ATile::GetTileStatus()
{
	return Status;
}

int32 ATile::GetOwner()
{
	return PlayerOwner;
}

void ATile::SetGridPosition(const double InX, const double InY)
{
	TileGridPosition.Set(InX, InY);
}

FVector2D ATile::GetGridPosition()
{
	return TileGridPosition;
}

// Setter per altezza della tile
void ATile::SetHeightLevel(int32 InHeightLevel)
{
	HeightLevel = FMath::Clamp(InHeightLevel, 0, 4);

	if (HeightLevel == 0)
	{
		TileType = ETileType::WATER;
		bWalkable = false;
	}
	else if (HeightLevel == 1)
	{
		TileType = ETileType::GROUND;
		bWalkable = true;
	}
	else if (HeightLevel == 2)
	{
		TileType = ETileType::HILL;
		bWalkable = true;
	}
	else if (HeightLevel == 3)
	{
		TileType = ETileType::MOUNTAIN;
		bWalkable = true;
	}
	else
	{
		TileType = ETileType::HIGHMOUNTAIN;
		bWalkable = true;
	}

	UpdateTileColor();
	UpdateTileHeight();
}

// Getter altezza tile
int32 ATile::GetHeightLevel() const
{
	return HeightLevel;
}

// Setter per tipo tile
void ATile::SetTileType(ETileType InTileType)
{
	TileType = InTileType;

	bWalkable = (TileType != ETileType::WATER && TileType != ETileType::TOWER);

	UpdateTileColor();
}

// Getter tipo tile
ETileType ATile::GetTileType() const
{
	return TileType;
}

// Funzione per fare il check se la tile è walkable
bool ATile::IsWalkable() const
{
	return bWalkable && Status == ETileStatus::EMPTY;
}

// Update colore tile in base al tipo
void ATile::UpdateTileColor()
{
	if (!DynamicMaterial)
	{
		return;
	}

	// Colore default
	FLinearColor TileColor = FLinearColor::White;

	// In base al tipo della tile cambio il colore
	switch (TileType)
	{
	case ETileType::WATER:
		TileColor = FLinearColor::Blue;
		break;

	case ETileType::GROUND:
		TileColor = FLinearColor::Green;
		break;

	case ETileType::HILL:
		TileColor = FLinearColor::Yellow;
		break;

	case ETileType::MOUNTAIN:
		TileColor = FLinearColor(1.f, 0.5f, 0.f);
		break;

	case ETileType::HIGHMOUNTAIN:
		TileColor = FLinearColor::Red;
		break;
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("TileColor"), TileColor);
}

// Update altezza tile in base al livello con moltiplicatore 50.0f
void ATile::UpdateTileHeight()
{
	FVector CurrentLocation = GetActorLocation();
	CurrentLocation.Z = HeightLevel * 50.0f;
	SetActorLocation(CurrentLocation);
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();

	if (StaticMeshComponent)
	{
		UMaterialInterface* BaseMaterial = StaticMeshComponent->GetMaterial(0);
		if (BaseMaterial)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			StaticMeshComponent->SetMaterial(0, DynamicMaterial);
		}
	}

	UpdateTileColor();
	UpdateTileHeight();
}

// Called every frame
//void ATile::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}
