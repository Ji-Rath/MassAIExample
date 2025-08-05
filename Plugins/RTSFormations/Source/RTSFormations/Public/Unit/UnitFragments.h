#pragma once
#include "FormationPresets.h"
#include "MassEntityElementTypes.h"
#include "UnitFragments.generated.h"

static int16 UnitNum = 0;

// Handle for units in the game
USTRUCT(BlueprintType)
struct FUnitHandle
{
	GENERATED_BODY()

	FUnitHandle()
	{
		UnitID = UnitNum++;
	};

	UPROPERTY()
	int16 UnitID;

	bool operator==(const FUnitHandle Other) const
	{
		return UnitID == Other.UnitID;
	}

	bool operator!=(const FUnitHandle Other) const
	{
		return !operator==(Other);
	}

	friend uint32 GetTypeHash(const FUnitHandle Entity)
	{
		return Entity.UnitID;
	}
};

USTRUCT()
struct FUnitSettings
{
	GENERATED_BODY()

	float InterpolationSpeed = 5.f;
	int FormationLength = 8; // The entity length of the 'front' of the unit
	float BufferDistance = 100.f;
	int Rings = 2; // Amount of rings for the circle formation
	EFormationType Formation = EFormationType::Rectangle; // The type of formation - WIP
	bool bHollow = false;
};

USTRUCT()
struct FUnitFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	FUnitFragment() = default;

	UPROPERTY()
	FUnitHandle UnitHandle;
	
	// Entities in the unit
	FVector3f UnitDestination = FVector3f::ZeroVector; // Destination of the unit
	FRotator3f UnitRotation;

	// Interpolated movement
	FVector3f InterpDestination = FVector3f::ZeroVector;
	FRotator3f InterpRotation;
	
	FVector2f ForwardDir;
	
	FUnitSettings UnitSettings;

	bool operator==(const FUnitFragment& OtherUnitFragment) const
	{
		return OtherUnitFragment.UnitHandle == UnitHandle;
	}

	bool operator==(const FUnitHandle& OtherUnitHandle) const
	{
		return OtherUnitHandle == UnitHandle;
	}
};
