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
	FLinearColor PlayerHumanColor = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);

	// Colore player AI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	FLinearColor PlayerAIColor = FLinearColor(0.7f, 0.0f, 1.0f, 1.0f);

	// Colore giallo per torre contesa
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	FLinearColor ContestedColor = FLinearColor(0.35f, 0.5f, 1.0f, 1.0f);

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	class AGameField* GameField;

	// Inizializzazione stato corrente della torre (da specifiche spawnata NEUTRAL)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	ETowerStatus CurrentStatus = ETowerStatus::NEUTRAL;

	// Funzione per aggiornare lo stato della torre, passo -1 siccome viene spawnata neutrale
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void UpdateTowerStatus(ETowerStatus NewStatus, int32 NewControllingPlayerID = -1);

	// PlayerID che controlla la torre (-1 = nessuno, 0 = human, 1 = AI)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	int32 ControllingPlayerID = -1;

	// Posizione della torre nella griglia così da poter calcolare la zona dove è possibile catturare la torre
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	FVector2D GridPosition;

	// Timer per il tick per controllare la torre ed assegnare il colore
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	float CaptureCheckTimer = 0.f;

	// Funzione per fare un check sulla zona di cattura, se ci sono unità human/AI allora aggiorno torre
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void CheckCaptureZone(class AGameField* GameField);

	// Funzione per ottenere tutte le unità nella zona cattura
	UFUNCTION(BlueprintCallable, Category = "Tower")
	TArray<class AUnit*> GetUnitsInCaptureZone(AGameField* GameField);

	// Setter per la posizione della torre nella griglia
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void SetGridPosition(FVector2D InPosition) { GridPosition = InPosition; }

	// Getter per la posizione della torre nella griglia
	UFUNCTION(BlueprintCallable, Category = "Tower")
	FVector2D GetGridPosition() const { return GridPosition; }

	// Getter per ControllingPlayerID
	UFUNCTION(BlueprintCallable, Category = "Tower")
	int32 GetControllingPlayerID() const { return ControllingPlayerID; }

	// Getter per CurrentState
	UFUNCTION(BlueprintCallable, Category = "Tower")
	ETowerStatus GetCurrentStatus() const { return CurrentStatus; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
