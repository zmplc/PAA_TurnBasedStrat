// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/InputComponent.h"
#include "GameField.h"
#include "Unit.generated.h"

// Classi delle unità
UENUM(BlueprintType)
enum class EUnitType : uint8
{
	SNIPER	UMETA(DisplayName = "Sniper"),
	BRAWLER UMETA(DisplayName = "Brawler")
};

// Tipo di attacco
UENUM(BlueprintType)
enum class EAttackType : uint8
{
	RANGED	UMETA(DisplayName = "Attacco a distanza"),
	MELEE	UMETA(DisplayName = "Attacco a corto raggio")
};

UCLASS(Abstract)
class PAA_TURNBASEDSTRAT_API AUnit : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AUnit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Proprietà base delle unità
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit Type")
	EUnitType UnitType;

	// Tipo di attacco: sniper è ranged, brawler è melee
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit Type")
	EAttackType AttackType;

	// Movimento: max 4 celle sniper, max 6 celle brawler
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit Stats")
	int32 MaxMovement;

	// Range attacco: max 10 sniper, 1 brawler
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit Stats")
	int32 AttackRange;

	// Danno minimo: 4 sniper, 1 brawler
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit Stats")
	int32 MinDamage;

	// Danno massimo: 8 sniper, 6 brawler
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit Stats")
	int32 MaxDamage;

	// Vita massima: 20 sniper, 40 brawler
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit Stats")
	int32 MaxHealth;

	// Vita attuale unità
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Stats")
	int32 CurrentHealth;

	// Chi è il proprietario dell'unità
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Owner")
	int32 OwnerPlayerID;

	// Se l'unità è stata mossa nel turno corrente
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Status")
	bool bHasMovedThisTurn;

	// Se l'unità ha attaccato nel turno corrente
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Status")
	bool bHasAttackedThisTurn;

	// Posizione corrente dell'unità, getter e setter, posizione iniziale per respawn, funzione respawn
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Position")
	FVector2D CurrentGridPosition;

	UFUNCTION(BlueprintCallable, Category = "Unit Position")
	FVector2D GetCurrentGridPosition() const { return CurrentGridPosition; }

	UFUNCTION(BlueprintCallable, Category = "Unit Position")
	void SetCurrentGridPosition(const FVector2D& NewPosition) { CurrentGridPosition = NewPosition; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Position")
	FVector2D InitialGridPosition;

	UFUNCTION(BlueprintCallable, Category = "Unit Combat")
	virtual void RespawnAtInitialPosition();

	// Mesh visivo
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* UnitRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* UnitMesh;

	// Metodi per movimento
	UFUNCTION(BlueprintCallable, Category = "Unit Movement")
	virtual bool CanMoveTo(int32 TargetX, int32 TargetY, const class AGameField* GameField) const;

	UFUNCTION(BlueprintCallable, Category = "Unit Movement")
	virtual bool MoveTo(int32 TargetX, int32 TargetY, class AGameField* GameField);

	UFUNCTION(BlueprintCallable, Category = "Unit Movement")
	virtual int32 GetMovementCost(int32 TargetX, int32 TargetY, const class AGameField* GameField) const;
	
	// Metodi per attacco
	UFUNCTION(BlueprintCallable, Category = "Unit Combat")
	virtual int32 CalculateDamage() const;

	UFUNCTION(BlueprintCallable, Category = "Unit Combat")
	virtual void ApplyDamage(int32 DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Unit Combat")
	virtual bool CanAttack(AUnit* TargetUnit, const class AGameField* GameField) const;

	UFUNCTION(BlueprintCallable, Category = "Unit Combat")
	bool IsAlive() const;
	
	// Metodi per turni
	UFUNCTION(BlueprintCallable, Category = "Unit Status")
	virtual void ResetTurnStatus();
};
