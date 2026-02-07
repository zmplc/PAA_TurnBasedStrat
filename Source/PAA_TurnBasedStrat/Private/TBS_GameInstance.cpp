// Fill out your copyright notice in the Description page of Project Settings.


#include "TBS_GameInstance.h"

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

