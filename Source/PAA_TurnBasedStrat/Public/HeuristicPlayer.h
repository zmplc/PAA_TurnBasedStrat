// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerInterface.h"
#include "TBS_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "HeuristicPlayer.generated.h"

UCLASS()
class PAA_TURNBASEDSTRAT_API AHeuristicPlayer : public APawn, public IPlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AHeuristicPlayer();

    UTBS_GameInstance* GameInstance;

    // PlayerID, per AI è sempre 1
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
    int32 PlayerID;

    // Turno del player
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
    bool IsMyTurn;

    // Classe Sniper da spawnare
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit")
    TSubclassOf<AUnit> SniperClass;

    // Classe Brawler da spawnare
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit")
    TSubclassOf<AUnit> BrawlerClass;

    // Classe da spawnare
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit")
    TSubclassOf<AUnit> ClassToSpawn;

    // Sniper piazzato: y/n
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
    bool bHasPlacedSniper;

    // Brawler piazzato: y/n
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
    bool bHasPlacedBrawler;

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Interfaccia
    virtual void OnPlacementTurnStart() override;
    virtual void OnTurnStart() override;
    virtual void OnTurnEnd() override;
    virtual void PerformTurnActions() override;
    virtual void OnWin() override;
    virtual void OnLose() override;

    // Funzione per piazzare le unità in modo automatico
    UFUNCTION()
    void PlaceUnitAutomatically();

    // Funzione per trovare una tile valida per il piazzamento in modo randomico in Y=22,23,24
    UFUNCTION()
    ATile* FindRandomValidTile(AGameField* GameField);

    // Di seguito definisco le funzioni per implementare algoritmi euristici
    // Funzione per trovare da tile start a tile goal usando algoritmo greedy best first search
    UFUNCTION()
    TArray<FIntPoint> FindPathGreedy(FIntPoint Start, FIntPoint Goal, AGameField* GameField);

    // Calcolo distanza manhattan tra due tile
    UFUNCTION()
    int32 ManhattanDistance(FIntPoint A, FIntPoint B);

    // Considero la torre migliore da raggiungere usando gli score calcolati con le funzioni euristiche
    UFUNCTION()
    ATower* EvaluateBestTower(AUnit* Unit, AGameField* GameField);

    // Considero il nemico da attaccare usando gli score calcolati con le funzioni euristiche
    UFUNCTION()
    AUnit* EvaluateBestTarget(AUnit* Attacker);

    // Funzione per calcolare lo score per la torre da raggiungere (considero lo stato della torre e la distanza)
    UFUNCTION()
    float EvaluateTowerScore(ATower* Tower, FVector2D UnitPos);

    // Funzione per calcolare lo score per il nemico da attaccare (considero gli HP, la classe e la distanza)
    UFUNCTION()
    float EvaluateEnemyScore(AUnit* Enemy, FVector2D UnitPos);

    // Di seguito riprendo le funzioni base dell'AI
    // Funzione per decidere il target prioritario
    UFUNCTION()
    FIntPoint DecideTarget(AUnit* Unit, AGameField* GameField);

    // Funzione per trovare tile camminabili nella zona di cattura
    UFUNCTION()
    FIntPoint FindWalkableTileCaptureZone(FVector2D TowerPos, AGameField* GameField);

    // Prossima mossa per arrivare al target
    UFUNCTION()
    FIntPoint NextMoveTowardsTarget(AUnit* Unit, FIntPoint TargetPos, AGameField* GameField);

    // Funzione per mostrare il range di movimento di una unità dell'AI
    UFUNCTION()
    void ShowMovementRange(AUnit* Unit, AGameField* GameField);

    // Funzione per nascondere il range di movimento dell'AI
    UFUNCTION()
    void HideMovementRange(AGameField* GameField);

    // Funzione per mostrare range movimento con timing delle unità dell'AI
    UFUNCTION()
    void ProcessUnit(TArray<AUnit*> Units, int32 CurrentIndex, ATBS_GameMode* GM);

private:
    FTimerHandle AI_TurnTimerHandle;

    // Array delle tile evidenziate per range movimento
    TArray<ATile*> HighlightedMovementTiles;

    // Tile corrente dell'unità per evidenziarla quando viene mostrato il range di movimento
    ATile* CurrentUnitTile;
};
