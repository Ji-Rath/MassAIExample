// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "PersistentDataFragment.generated.h"

// Shared fragment which lets the system know which template to use when loading the entity
USTRUCT()
struct FPersistentDataFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UMassEntityConfigAsset> EntityConfig;
};

// Simple tag that defines an entity should be persisted
USTRUCT()
struct FPersistentDataTag : public FMassTag
{
	GENERATED_BODY()
	
};

// Proxy struct for the FTransformFragment
// Since the transform is marked as transient, the FTransformFragment does not get saved/loaded properly
USTRUCT()
struct FPersistentTransformFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform;
};

// Persist entity data using the SaveGame system within unreal engine
UCLASS()
class UPersistentDataTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FPersistentDataFragment PersistentDataFragment;
	
	void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
