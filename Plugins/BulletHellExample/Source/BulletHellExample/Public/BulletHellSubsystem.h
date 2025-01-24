// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSubsystemBase.h"
#include "MassProcessor.h"
#include "BulletHellSubsystem.generated.h"

namespace BulletHell::Signals
{
	const FName BulletSpawned = FName(TEXT("BulletSpawned"));
	const FName BulletDestroy = FName(TEXT("BulletDestroy"));
}

class UMassEntityConfigAsset;
/**
 * Subsystem which manages communication between mass bullet hell processors and gameplay systems
 */
UCLASS()
class BULLETHELLEXAMPLE_API UBulletHellSubsystem : public UMassTickableSubsystemBase
{
	GENERATED_BODY()

public:
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
};

template<>
struct TMassExternalSubsystemTraits<UBulletHellSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
