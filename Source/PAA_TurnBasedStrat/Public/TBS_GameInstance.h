// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TBS_GameInstance.generated.h"

class APawn;

// Struct per una singola entry dello storico delle mosse con il relativo testa collegato a attacco/movimento o entrambi
USTRUCT(BlueprintType)
struct FMoveHistoryEntry
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Move History")
	FString MoveText;
	FMoveHistoryEntry() : MoveText(TEXT("")) {}
	FMoveHistoryEntry(const FString& Text) : MoveText(Text) {}
};

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

	// Vita unità HumanPlayer
	UPROPERTY(BlueprintReadWrite, Category = "Unit Stats")
	int32 HumanSniperHP = 20;

	UPROPERTY(BlueprintReadWrite, Category = "Unit Stats")
	int32 HumanBrawlerHP = 40;

	// Vita unità AI
	UPROPERTY(BlueprintReadWrite, Category = "Unit Stats")
	int32 AiSniperHP = 20;

	UPROPERTY(BlueprintReadWrite, Category = "Unit Stats")
	int32 AiBrawlerHP = 40;

	// Storico mosse (metto TArray di struct con la singola entry)
	UPROPERTY(BlueprintReadOnly, Category = "Move History")
	TArray<FMoveHistoryEntry> MoveHistory;

	// AIPlayer selezionata dal giocatore nel MainMenu (0=RandomPlayer, 1=HeuristicPlayer)
	UPROPERTY(BlueprintReadWrite, Category = "AI")
	int32 SelectedAIType = 0;

	// Numero delle torri controllate da HumanPlayer
	UPROPERTY(BlueprintReadOnly, Category = "Towers")
	int32 HumanTowersControlled = 0;

	// Numero delle torri controllate dall'AI
	UPROPERTY(BlueprintReadOnly, Category = "Towers")
	int32 AiTowersControlled = 0;

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
	
	// Funzione per aggiornare gli HP delle unità
	UFUNCTION(BlueprintCallable, Category = "Unit Stats")
	void UpdateUnitHP(int32 PlayerID, bool bIsSniper, int32 NewHP);

	// Funzione per aggiungere una mossa allo storico delle mosse
	UFUNCTION(BlueprintCallable, Category = "Move History")
	void AddMoveToHistory(const FString& MoveText);

	// Getter per ottenere lo storico delle mosse
	UFUNCTION(BlueprintCallable, Category = "Move History")
	TArray<FString> GetMoveHistory() const;

	// Funzione per resettare lo storico delle mosse quando finisce la partita
	UFUNCTION(BlueprintCallable, Category = "Move History")
	void ClearMoveHistory();

	// Funzione per aggiornare il numero delle torri controllate dai player
	UFUNCTION(BlueprintCallable, Category = "Towers")
	void UpdateTowerCount(int32 HumanCount, int32 AiCount);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ResetGame();
};
