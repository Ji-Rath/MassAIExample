// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CollisionSubsystem.h"
#include "MassEntityTypes.h"
#include "CollisionFragments.generated.h"

namespace HashGridExample::Signals
{
	const FName EntityQueried = FName(TEXT("EntityQueried"));
}

/**
 * 
 */
USTRUCT()
struct ENTITYCOLLISION_API FCollisionFragment : public FMassFragment
{
	GENERATED_BODY()
public:
	FHashGridExample::FCellLocation CellLocation;
};
