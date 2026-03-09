// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CollisionSubsystem.h"
#include "MassEntityTraitBase.h"
#include "CollisionFragments.generated.h"

namespace HashGridExample::Signals
{
	const FName EntityQueried = FName(TEXT("EntityQueried"));
}

// We need to pick a relatively high # so that moving entities will be considered
constexpr int8 MaxObstacleResults = 15;

// Stores location information so that the entity position in the hash grid can be updated
USTRUCT()
struct ENTITYCOLLISION_API FCollisionFragment : public FMassFragment
{
	GENERATED_BODY()
public:
	FHashGridExample::FCellLocation CellLocation;
};

// Settings related to ORCA avoidance
USTRUCT()
struct ENTITYCOLLISION_API FAvoidanceSettings : public FMassConstSharedFragment
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Avoidance")
	float ObstacleSearchDistance = 100.f;
	
	UPROPERTY(EditAnywhere, Category = "Avoidance")
	float TimeHorizon = 2.f;
	
	UPROPERTY(EditAnywhere, Category = "Avoidance")
	float PushStrength = 100.f;
};

// Defines an entity that will avoid obstacles
USTRUCT()
struct ENTITYCOLLISION_API FEntityAvoidanceTag : public FMassTag
{
	GENERATED_BODY()
};

// Defines an entity that has collision (Other entities will avoid it)
USTRUCT()
struct ENTITYCOLLISION_API FObstacleTag : public FMassTag
{
	GENERATED_BODY()
};

UCLASS()
class UAvoidanceTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
	
	UPROPERTY(EditAnywhere, Category = "Avoidance")
	FAvoidanceSettings AvoidanceSettings;
};


UCLASS()
class UObstacleTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

