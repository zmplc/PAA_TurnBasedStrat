// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tower.generated.h"

// Stato della torre: neutrale, sotto controllo, contesa
UENUM()
enum class ETowerStatus : uint8
{
	NEUTRAL     UMETA(DisplayName = "Neutral"),
	CONTROLLED  UMETA(DisplayName = "Controlled"),
	CONTESTED	UMETA(DisplayName = "Contested")
};

UCLASS()
class PAA_TURNBASEDSTRAT_API ATower : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATower();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* TowerMesh;

	// Colore grigio chiaro per stato neutrale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	FLinearColor NeutralColor = FLinearColor(0.8f, 0.8f, 0.8f);

	// Colore player umano
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	FLinearColor PlayerHumanColor = FLinearColor(0.0f, 1.0f, 1.0f);

	// Colore player AI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	FLinearColor PlayerAIColor = FLinearColor(1.0f, 0.0f, 1.0f);

	// Colore giallo per torre contesa
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	FLinearColor ContestedColor = FLinearColor(1.0f, 1.0f, 0.0f);

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	// Inizializzazione stato corrente della torre (da specifiche spawnata NEUTRAL)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	ETowerStatus CurrentState = ETowerStatus::NEUTRAL;

	// Funzione per aggiornare lo stato della torre, passo -1 siccome viene spawnata neutrale
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void UpdateTowerStatus(ETowerStatus NewState, int32 ControllingPlayerID = -1);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
