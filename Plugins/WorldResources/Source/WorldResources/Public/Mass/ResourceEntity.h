// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MassEntityTypes.h"
#include "ResourceEntity.generated.h"

// Defines simple properties for an open simulation entity
USTRUCT()
struct FResourceUserFragment : public FMassFragment
{
	GENERATED_BODY()
	
	// Tags that the entity currently holds
	FGameplayTagContainer Tags;
};
