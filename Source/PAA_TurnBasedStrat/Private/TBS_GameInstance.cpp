// Fill out your copyright notice in the Description page of Project Settings.


#include "TBS_GameInstance.h"
#include "Kismet/GameplayStatics.h"

UTBS_GameInstance::UTBS_GameInstance()
{
    // Valori di default
    ScoreHumanPlayer = 0;
    ScoreAiPlayer = 0;
    CurrentTurnMessage = "Messaggio";
}

void UTBS_GameInstance::IncrementScoreHumanPlayer()
{
    ScoreHumanPlayer += 1;
}

void UTBS_GameInstance::IncrementScoreAiPlayer()
{
    ScoreAiPlayer += 1;
}

int32 UTBS_GameInstance::GetScoreHumanPlayer()
{
    return ScoreHumanPlayer;
}

int32 UTBS_GameInstance::GetScoreAiPlayer()
{
    return ScoreAiPlayer;
}

void UTBS_GameInstance::ResetScores()
{
    ScoreHumanPlayer = 0;
    ScoreAiPlayer = 0;
}

FString UTBS_GameInstance::GetTurnMessage() const
{
    return CurrentTurnMessage;
}

void UTBS_GameInstance::SetTurnMessage(const FString& Message)
{
    CurrentTurnMessage = Message;
}

FString UTBS_GameInstance::GetTurnOwner() const
{
    return CurrentTurnOwner;
}

void UTBS_GameInstance::SetTurnOwner(const FString& Owner)
{
    CurrentTurnOwner = Owner;
}

void UTBS_GameInstance::UpdateUnitHP(int32 PlayerID, bool bIsSniper, int32 NewHP)
{
	// HumanPlayer
	if (PlayerID == 0)
	{
		if (bIsSniper)
		{
			HumanSniperHP = NewHP;
		}
		else
		{
			HumanBrawlerHP = NewHP;
		}
	}
	// AI
	else if (PlayerID == 1)
	{
		if (bIsSniper)
		{
			AiSniperHP = NewHP;
		}
		else
		{
			AiBrawlerHP = NewHP;
		}
	}
}

void UTBS_GameInstance::AddMoveToHistory(const FString& MoveText)
{
	// Esempio di testo per storico: HP: S B4 -> D6
	// Creo entry per array
	FMoveHistoryEntry Entry(MoveText);
	// Aggiungo l'entry creata
	MoveHistory.Add(Entry);
	// Notifico il widget che è stata aggiunta una mossa allo storico
	OnMoveAdded.Broadcast();
}

TArray<FString> UTBS_GameInstance::GetMoveHistory() const
{
	// Ho bisogno di questa funzione per passare al widget solo un array di stringhe siccome non accetta una struct
	// Creo array vuoto per mettere dentro i testi delle mosse
	TArray<FString> Result;
	// Per ogni entry salvato in MoveHistory prendo il suo testo e lo metto in Result
	for (const FMoveHistoryEntry& Entry : MoveHistory)
	{
		Result.Add(Entry.MoveText);
	}
	// Faccio return di array Result
	return Result;
}

void UTBS_GameInstance::ClearMoveHistory()
{
	// Svuoto l'array
	MoveHistory.Empty();
}

void UTBS_GameInstance::UpdateTowerCount(int32 HumanTowerCount, int32 AiTowerCount)
{
	HumanTowersControlled = HumanTowerCount;
	AiTowersControlled = AiTowerCount;
}

void UTBS_GameInstance::ResetMapConfig()
{
	// Reset ai valori default
	Level0Threshold = 0.42f;
	Level1Threshold = 0.46f;
	Level2Threshold = 0.54f;
	Level3Threshold = 0.64f;
	Level4Threshold = 0.74f;

	UE_LOG(LogTemp, Log, TEXT("GameInstance: Parametri generazione mappa resettati ai valori default"));
}

void UTBS_GameInstance::ResetGame()
{
	HumanSniperHP = 20;
	HumanBrawlerHP = 40;
	AiSniperHP = 20;
	AiBrawlerHP = 40;
	HumanTowersControlled = 0;
	AiTowersControlled = 0;
	ClearMoveHistory();
	ResetMapConfig();
}

void UTBS_GameInstance::ResetLevel()
{
	ResetGame();
	// Ritorno al MainMenu
	UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}

