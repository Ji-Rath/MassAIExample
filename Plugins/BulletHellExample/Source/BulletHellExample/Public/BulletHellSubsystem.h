// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HierarchicalHashGrid2D.h"
#include "MassSubsystemBase.h"
#include "MassProcessor.h"
#include "BulletHellSubsystem.generated.h"

namespace BulletHell::Signals
{
	const FName BulletSpawned = FName(TEXT("BulletSpawned"));
	const FName BulletDestroy = FName(TEXT("BulletDestroy"));
}

typedef THierarchicalHashGrid2D<2, 4, FMassEntityHandle> FBHEntityHashGrid;	// 2 levels of hierarchy, 4 ratio between levels

class UMassEntityConfigAsset;
/**
 * Subsystem which manages communication between mass bullet hell processors and gameplay systems
 */
UCLASS()
class BULLETHELLEXAMPLE_API UBulletHellSubsystem : public UMassTickableSubsystemBase
{
	GENERATED_BODY()

public:
	const FBHEntityHashGrid& GetHashGrid() const;
	FBHEntityHashGrid& GetHashGrid_Mutable();
	
	void GetPlayerLocation(FVector& OutLocation) const;

	UFUNCTION(BlueprintCallable)
	void SpawnBullet(UMassEntityConfigAsset* BulletConfig, const FVector& Location, const FVector& Direction);

	UPROPERTY(EditAnywhere)
	UMassEntityConfigAsset* BulletConfigAsset;

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual TStatId GetStatId() const override;

private:
	FVector PlayerLocation;

	UPROPERTY()
	APawn* CachedPlayerPawn;

	FBHEntityHashGrid EntityHashGrid;
};

template<>
struct TMassExternalSubsystemTraits<UBulletHellSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
