// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TBS_GameInstance.generated.h"

class APawn;
/**
 * 
 */
UCLASS()
class PAA_TURNBASEDSTRAT_API UTBS_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UTBS_GameInstance();

	// score value for human player
	UPROPERTY(EditAnywhere, Category = "Score")
	int32 ScoreHumanPlayer = 0;

	// score value for AI player
	UPROPERTY(EditAnywhere, Category = "Score")
	int32 ScoreAiPlayer = 0;

	// message to show every turn
	UPROPERTY(EditAnywhere, Category = "UI")
	FString CurrentTurnMessage = "Message";

	// Messaggio per mostrare di chi è il turno
	UPROPERTY(EditAnywhere, Category = "UI")
	FString CurrentTurnOwner = "Current Player";

	// Incremento vittorie HumanPlayer
	UFUNCTION(BlueprintCallable, Category = "Score")
	void IncrementScoreHumanPlayer();

	// Incremento vittorie AIPlayer
	UFUNCTION(BlueprintCallable, Category = "Score")
	void IncrementScoreAiPlayer();

	// get the score for human player
	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetScoreHumanPlayer();

	// get the score for AI player
	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetScoreAiPlayer();

	// Reset punteggi
	UFUNCTION(BlueprintCallable, Category = "Score")
	void ResetScores();

	// Getter messaggio
	UFUNCTION(BlueprintCallable, Category = "UI")
	FString GetTurnMessage() const;

	// Setter messaggio
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetTurnMessage(const FString& Message);

	// Getter turno
	UFUNCTION(BlueprintCallable, Category = "UI")
	FString GetTurnOwner() const;

	// Setter turno
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetTurnOwner(const FString& Owner);
};
