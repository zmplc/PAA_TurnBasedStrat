// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ConfigData.generated.h"

/**
 * 
 */
UCLASS()
class PAA_TURNBASEDSTRAT_API UConfigData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public: 
	// grid size 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	int32 GridSize;

	// tile size
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	float TileSize;

	// tile padding percentage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	float CellPadding;
};
