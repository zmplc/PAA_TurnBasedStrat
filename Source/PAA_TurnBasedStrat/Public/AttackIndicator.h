// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BillboardComponent.h"
#include "AttackIndicator.generated.h"

UCLASS()
class PAA_TURNBASEDSTRAT_API AAttackIndicator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAttackIndicator();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* IconMesh;

	// Setto il  target sopra il quale posizionare l'icona dell'attacco
	void SetTargetUnit(class AUnit* TargetUnit);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Unità che è bersagio dell'attacco da seguire e sopra la quale posizionare l'icona dell'attacco
	UPROPERTY()
	class AUnit* Target;
};
