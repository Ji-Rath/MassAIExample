// Fill out your copyright notice in the Description page of Project Settings.


#include "AdvancedRandomMovementTrait.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"
#include "MassMovement/Public/MassMovementFragments.h"

void UAdvancedRandomMovementTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FNPC>();
}

UAdvancedRandomMovementProcessor::UAdvancedRandomMovementProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UAdvancedRandomMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, ([this](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> TransformsList = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMassMoveTargetFragment> NavTargetsList = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		const FMassMovementParameters& MovementParameters = Context.GetConstSharedFragment<FMassMovementParameters>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FTransform& Transform = TransformsList[EntityIndex].GetTransform();
			FMassMoveTargetFragment& MoveTarget = NavTargetsList[EntityIndex];

			FVector CurrentLocation = Transform.GetLocation();
			FVector TargetVector = MoveTarget.Center - Transform.GetLocation();
			TargetVector.Z = 0.f; 

			MoveTarget.DistanceToGoal = (TargetVector).Size();
			MoveTarget.Forward = (TargetVector).GetSafeNormal();

			if (MoveTarget.DistanceToGoal <= 20.f || MoveTarget.Center == FVector::ZeroVector)
			{
				MoveTarget.Center = FVector(FMath::RandRange(-1.f, 1.f) * 1000.f, FMath::RandRange(-1.f, 1.f) * 1000.f, CurrentLocation.Z);
				MoveTarget.DistanceToGoal = (MoveTarget.Center - Transform.GetLocation()).Size();
				MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();
				MoveTarget.DesiredSpeed = FMassInt16Real(MovementParameters.DefaultDesiredSpeed);
			}
		}
	}));
}

void UAdvancedRandomMovementProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FNPC>(EMassFragmentPresence::All);
}
