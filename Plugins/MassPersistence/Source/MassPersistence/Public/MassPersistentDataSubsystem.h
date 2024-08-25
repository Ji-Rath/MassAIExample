// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassExternalSubsystemTraits.h"
#include "MassSaveGame.h"
#include "MassSubsystemBase.h"
#include "MassPersistentDataSubsystem.generated.h"

namespace PersistentData::Signals
{
	const FName SaveEntity = FName(TEXT("SaveEntity"));
	const FName RandomizePositions = FName(TEXT("RandomizePositions"));
	const FName EntityLoaded = FName(TEXT("EntityLoaded"));
}

struct FMassEntityHandle;
class USaveGame;
/**
 * 
 */
UCLASS()
class MASSPERSISTENCE_API UMassPersistentDataSubsystem : public UMassSubsystemBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SaveEntities(const FString& SlotName);
	
	void SaveEntityData(TArray<FMassEntityHandle>& Entities);

	UFUNCTION(BlueprintCallable)
	UMassSaveGame* FindOrCreateSaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadEntitiesFromSave(UMassSaveGame* SaveGameFile);

	UFUNCTION(BlueprintCallable)
	void SpawnEntities(UMassEntityConfigAsset* ConfigAsset, int Amount);

	UFUNCTION(BlueprintCallable)
	void RandomizePositions();

	UFUNCTION(BlueprintCallable)
	void ClearPersistedEntities();

	UPROPERTY(BlueprintReadWrite)
	UMassSaveGame* SaveGame;

	UPROPERTY()
	TArray<FMassEntityHandle> ManagedEntities;
};

template<>
struct TMassExternalSubsystemTraits<UMassPersistentDataSubsystem>
{
	enum
	{
		GameThreadOnly = false
	};
};
