// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerInterface.h"
#include "TBS_GameInstance.h"
#include "TBS_GameMode.h"
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

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual void OnPlacementTurnStart() override;
    virtual void OnTurnStart() override;
    virtual void OnTurnEnd() override;
    virtual void OnWin() override;
    virtual void OnLose() override;

private:
    FTimerHandle AI_TurnTimerHandle;

};
