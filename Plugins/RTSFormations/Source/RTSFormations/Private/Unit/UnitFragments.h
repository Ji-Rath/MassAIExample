#pragma once
#include "FormationPresets.h"
#include "MassEntityElementTypes.h"
#include "MassEntityHandle.h"
#include "UnitFragments.generated.h"

// Defines a unit entity
struct FUnitTag : public FMassTag
{
	
};

USTRUCT()
struct FUnitFragment : public FMassFragment
{
	GENERATED_BODY()
	
	// Entities in the unit
	TSet<FMassEntityHandle> Entities;
	
	TMap<int, FVector> NewPositions;
	
	FVector UnitPosition = FVector::ZeroVector;

	// The direction to turn the unit when rotating
	float TurnDirection = 1.f;

	// The entity length of the 'front' of the unit
	int FormationLength = 8;
	
	float BufferDistance = 100.f;

	// The type of formation - WIP
	EFormationType Formation = EFormationType::Rectangle;
	
	// Amount of rings for the circle formation
	int Rings = 2;
	
	bool bHollow = false;

	UPROPERTY()
	FVector FarCorner;

	// Interpolated movement
	UPROPERTY()
	FVector InterpolatedDestination = FVector::ZeroVector;

	UPROPERTY()
	FRotator Rotation;
	
	UPROPERTY()
	FRotator InterpRotation;
	
	UPROPERTY()
	FRotator OldRotation;

	UPROPERTY()
	float InterpolationSpeed = 5.f;

	UPROPERTY()
	bool bBlendAngle = false;

	UPROPERTY()
	FVector ForwardDir;
};
