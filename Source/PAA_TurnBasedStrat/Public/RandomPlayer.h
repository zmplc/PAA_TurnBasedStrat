// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerInterface.h"
#include "TBS_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "RandomPlayer.generated.h"

class AUnit;

UCLASS()
class PAA_TURNBASEDSTRAT_API ARandomPlayer : public APawn, public IPlayerInterface
{
	GENERATED_BODY()

public:
    ARandomPlayer();

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
    virtual bool PendingTurnActions() const override;
    virtual void OnWin() override;
    virtual void OnLose() override;

    // Funzione per piazzare le unità in modo automatico
    UFUNCTION()
    void PlaceUnitAutomatically();

    // Funzione per trovare una tile valida per il piazzamento in modo randomico in Y=22,23,24
    UFUNCTION()
    ATile* FindRandomValidTile(AGameField* GameField);

private:
    FTimerHandle AI_TurnTimerHandle;

};
