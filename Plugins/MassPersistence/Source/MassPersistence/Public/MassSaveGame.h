// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MassSaveGame.generated.h"

class UMassEntityConfigAsset;
struct FInstancedStruct;

// Struct data which defines an entity and its persisted fragments
USTRUCT()
struct FEntitySaveData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FInstancedStruct> EntityFragments;
	
	UPROPERTY()
	UMassEntityConfigAsset* ConfigAsset;

};
/**
 * Stores data related to entity data that is persisted
 */
UCLASS()
class MASSPERSISTENCE_API UMassSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	// Entities that have been persisted
	UPROPERTY()
	TArray<FEntitySaveData> Entities;
};
