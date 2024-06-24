// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ResourceData.generated.h"

class USmartObjectDefinition;

/**
 * Defines simple resource data parameters
 */
USTRUCT(BlueprintType)
struct FResourceData : public FTableRowBase
{
	GENERATED_BODY();
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Resource)
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Resource, meta=(Categories="Resource"))
	FGameplayTag ResourceType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Resource)
	USmartObjectDefinition* SODefinition;

	FResourceData() = default;
};
