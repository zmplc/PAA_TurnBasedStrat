// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit.h"
#include "UnitBrawler.generated.h"

/**
 * 
 */
UCLASS()
class PAA_TURNBASEDSTRAT_API AUnitBrawler : public AUnit
{
	GENERATED_BODY()
	
public:
	AUnitBrawler()
	{
		UnitType = EUnitType::BRAWLER;
		AttackType = EAttackType::MELEE;
		MaxMovement = 6;
		AttackRange = 1;
		MinDamage = 1;
		MaxDamage = 6;
		MaxHealth = 40;
		CurrentHealth = 40;
	}
};
