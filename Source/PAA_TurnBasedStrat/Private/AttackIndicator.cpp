// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackIndicator.h"
#include "Unit.h"

// Sets default values
AAttackIndicator::AAttackIndicator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// template function that creates a components
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	IconMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IconMesh"));

	// every actor has a RootComponent that defines the transform in the World
	SetRootComponent(Scene);
	IconMesh->SetupAttachment(Scene);

	// Siccome è solo un indicatore visivo tolgo tutte le collisioni e lo rendo non cliccabile
	IconMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	IconMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	// Inizializzo il target a puntatore nullo
	Target = nullptr;
}

// Called when the game starts or when spawned
void AAttackIndicator::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AAttackIndicator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Per ogni frame verifico se il target è vivo/valido, se non lo è allora faccio destroy dell'icona dell'attacco
	if (!Target || !IsValid(Target) || !Target->IsAlive())
	{
		Destroy();
	}
}

void AAttackIndicator::SetTargetUnit(AUnit* TargetUnit)
{
	// Imposto il target che bisogna seguire per poi posizionare icona
	Target = TargetUnit;
	// Posiziono icona sopra target
	if (Target)
	{
		FVector TargetLocation = Target->GetActorLocation();
		TargetLocation.Z += 300.0f;
		SetActorLocation(TargetLocation);
	}
}