// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

UENUM()
enum class EPlayerSide : uint8
{
	Human,
	AI
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PAA_TURNBASEDSTRAT_API IPlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 0 human, 1 AI
	int32 PlayerID;
	// Side del player
	EPlayerSide PlayerSide;
	//Colore del player (per torri e unità)
	FLinearColor PlayerColor;

	// Metodi
	// Turno piazzamento
	virtual void OnPlacementTurnStart() {};
	// Inizio turno: se umano attivo input/click, se AI calcolo le mosse con algoritmo
	virtual void OnTurnStart() {};
	// Fine turno: deseleziono le unità, disabilito input/click
	virtual void OnTurnEnd() {};
	// Eseguo le azioni del turno (movimento e attacco)
	virtual void PerformTurnActions() {};
	// Verifico se ci sono ancora unità da muovere/attaccare (in questo caso il turno viene terminato automaticamente dopo l'ultima azione)
	virtual bool PendingTurnActions() const { return false; };
	// Seleziono unità (solo per player umano)
	virtual void SelectUnit(class AUnit* Unit) {};
	// Vittoria/Sconfitta
	virtual void OnWin() {};
	virtual void OnLose() {};
};
