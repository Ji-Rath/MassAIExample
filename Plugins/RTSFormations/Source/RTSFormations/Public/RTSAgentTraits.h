// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassMovementTypes.h"
#include "RTSAgentSubsystem.h"
#include "Unit/UnitFragments.h"
#include "RTSAgentTraits.generated.h"

class URTSFormationSubsystem;

// Store basic info about the unit
USTRUCT()
struct RTSFORMATIONS_API FRTSFormationAgent : public FMassFragment
{
	GENERATED_BODY()

	FRTSFormationAgent() = default;
	
	// The offset of the entity within the unit
	FVector3f Offset;
};

USTRUCT()
struct FRTSCellLocFragment : public FMassFragment
{
	GENERATED_BODY()

	FRTSCellLocFragment() = default;
	
	RTSAgentHashGrid2D::FCellLocation CellLoc;
};

USTRUCT()
struct RTSFORMATIONS_API FRTSFormationSettings : public FMassConstSharedFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Formation")
	FMassMovementStyleRef WalkMovement;

	UPROPERTY(EditAnywhere, Category = "Formation")
	FMassMovementStyleRef RunMovement;
};

// Provides entity with FRTSFormationAgent fragment to enable formations
UCLASS()
class RTSFORMATIONS_API URTSFormationAgentTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere)
	FRTSFormationSettings FormationSettings;
};
