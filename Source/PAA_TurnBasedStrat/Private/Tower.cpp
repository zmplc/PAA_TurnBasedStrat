// Fill out your copyright notice in the Description page of Project Settings.


#include "Tower.h"

// Sets default values
ATower::ATower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// template function that creates a components
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));

	// every actor has a RootComponent that defines the transform in the World
	SetRootComponent(Scene);
	TowerMesh->SetupAttachment(Scene);
}

// Called when the game starts or when spawned
void ATower::BeginPlay()
{
	Super::BeginPlay();

	if (TowerMesh)
	{
		UMaterialInterface* BaseMaterial = TowerMesh->GetMaterial(0);
		if (BaseMaterial)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			TowerMesh->SetMaterial(0, DynamicMaterial);

			// Imposto stato iniziale della torre
			UpdateTowerStatus(ETowerStatus::NEUTRAL);
		}
	}
	
}

// Funzione per fare l'update dello stato della torre
void ATower::UpdateTowerStatus(ETowerStatus NewState, int32 ControllingPlayerID)
{
	CurrentState = NewState;
	if (DynamicMaterial)
	{
		FLinearColor TargetColor = FLinearColor::White;
		switch (CurrentState)
		{
		// Stato A: neutrale
		case ETowerStatus::NEUTRAL:
			TargetColor = NeutralColor;
			break;
		// Stato B: sotto controllo
		case ETowerStatus::CONTROLLED:
			// Controllato dal player umano
			if (ControllingPlayerID == 0)
			{
				TargetColor = PlayerHumanColor;
			}
			// Controllato da player AI
			else if (ControllingPlayerID == 1)
			{
				TargetColor = PlayerAIColor;
			}
			break;
		// Stato C: contesa
		case ETowerStatus::CONTESTED:
			TargetColor = ContestedColor;
			break;
		default:
			break;
		}
		// Aggiorno il colore della torre con il TargetColor
		DynamicMaterial->SetVectorParameterValue(FName("TowerColor"), TargetColor);
	}
	else {
		return;
	}
}

// Called every frame
void ATower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

