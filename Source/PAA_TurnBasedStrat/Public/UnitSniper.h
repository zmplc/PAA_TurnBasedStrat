// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit.h"
#include "UnitSniper.generated.h"

/**
 * 
 */
UCLASS()
class PAA_TURNBASEDSTRAT_API AUnitSniper : public AUnit
{
    GENERATED_BODY()

public:
    AUnitSniper()
    {
        UnitType = EUnitType::SNIPER;
        AttackType = EAttackType::RANGED;
        MaxMovement = 4;
        AttackRange = 10;
        MinDamage = 4;
        MaxDamage = 8;
        MaxHealth = 20;
        CurrentHealth = 20;
    }
};
