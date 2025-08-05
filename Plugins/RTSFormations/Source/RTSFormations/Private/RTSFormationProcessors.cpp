#include "RTSFormationProcessors.h"

#include "RTSFormationSubsystem.h"
#include "LaunchEntityProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavigationTypes.h"
#include "MassSignalSubsystem.h"
#include "MassSimulationLOD.h"
#include "RTSAgentTraits.h"
#include "RTSSignals.h"
#include "Engine/World.h"
#include "Unit/UnitFragments.h"

//----------------------------------------------------------------------//
//  URTSFormationInitializer
//----------------------------------------------------------------------//
URTSFormationInitializer::URTSFormationInitializer()
	: EntityQuery(*this)
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void URTSFormationInitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::None);
	EntityQuery.AddSharedRequirement<FUnitFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URTSFormationSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URTSFormationInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TArray<FUnitHandle> UnitHandles;
	
	// First query is to give all units an appropriate unit index.
	EntityQuery.ForEachEntityChunk(Context, [&UnitHandles](FMassExecutionContext& Context)
	{
		auto& UnitFragment = Context.GetSharedFragment<FUnitFragment>();

		UnitHandles.Emplace(UnitFragment.UnitHandle);
	});

	auto FormationSubsystem = UWorld::GetSubsystem<URTSFormationSubsystem>(EntityManager.GetWorld());
	for (const FUnitHandle& UnitHandle : UnitHandles)
	{
		FormationSubsystem->UpdateUnitPosition(UnitHandle);
	}
}

//----------------------------------------------------------------------//
//  URTSFormationDestroyer
//----------------------------------------------------------------------//
URTSFormationDestroyer::URTSFormationDestroyer()
	: EntityQuery(*this)
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

void URTSFormationDestroyer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::None);
	EntityQuery.AddSharedRequirement<FUnitFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URTSFormationSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URTSFormationDestroyer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	auto& FormationSubsystem = Context.GetMutableSubsystemChecked<URTSFormationSubsystem>();
	TArray<FUnitHandle> UnitSignals;
	
	EntityQuery.ForEachEntityChunk(Context, [&UnitSignals](FMassExecutionContext& Context)
	{
		auto UnitFragment = Context.GetSharedFragment<FUnitFragment>();

		// Signal affected units/entities at the end
		UnitSignals.AddUnique(UnitFragment.UnitHandle);
	});
	
	// Signal affected units/entities
	for(const auto& Unit : UnitSignals)
	{
		// Really the only time we should notify every entity in the unit is when the center point changes
		// Every other time we just have to notify the entity that is replacing the destroyed one
		FormationSubsystem.UpdateUnitPosition(Unit);
	}
}

//----------------------------------------------------------------------//
//  URTSAgentMovement
//----------------------------------------------------------------------//
void URTSAgentMovement::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FLaunchEntityFragment>(EMassFragmentAccess::None, EMassFragmentPresence::None);
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	EntityQuery.AddConstSharedRequirement<FRTSFormationSettings>();
	EntityQuery.AddSharedRequirement<FUnitFragment>(EMassFragmentAccess::ReadOnly);
	
	EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.SetChunkFilter(&FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void URTSAgentMovement::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		TConstArrayView<FRTSFormationAgent> RTSFormationAgents = Context.GetFragmentView<FRTSFormationAgent>();

		const FRTSFormationSettings& FormationSettings = Context.GetConstSharedFragment<FRTSFormationSettings>();
		const FMassMovementParameters& MovementParameters = Context.GetConstSharedFragment<FMassMovementParameters>();
		auto& UnitFragment = Context.GetSharedFragment<FUnitFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
			const FTransform& Transform = TransformFragments[EntityIndex].GetTransform();
			
			const FRTSFormationAgent& RTSFormationAgent = RTSFormationAgents[EntityIndex];

			auto Offset = RTSFormationAgent.Offset;
			Offset = Offset.RotateAngleAxis(UnitFragment.InterpRotation.Yaw, FVector3f(0.f,0.f,1.f));
			
			MoveTarget.Center = FVector(UnitFragment.InterpDestination + Offset);
			
			// Update move target values
			auto DiffToGoal = MoveTarget.Center - Transform.GetLocation();
			MoveTarget.DistanceToGoal = DiffToGoal.Length();
			MoveTarget.Forward = DiffToGoal.GetSafeNormal();
			
			// Once we are close enough to our goal, begin walking
			if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
			{
				//MoveTarget.CreateNewAction(EMassMovementAction::Stand, *GetWorld());
				MoveTarget.DesiredSpeed = FMassInt16Real(MovementParameters.GenerateDesiredSpeed(FormationSettings.WalkMovement, Context.GetEntity(EntityIndex).Index));
			}
		}
	});
}

//----------------------------------------------------------------------//
//  URTSFormationUpdate
//----------------------------------------------------------------------//
void URTSFormationUpdate::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	auto SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, RTS::Unit::Signals::FormationUpdated);
}

void URTSFormationUpdate::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	EntityQuery.AddConstSharedRequirement<FRTSFormationSettings>();
}

void URTSFormationUpdate::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	// Query to calculate move target for entities based on unit index
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();

		const FRTSFormationSettings& FormationSettings = Context.GetConstSharedFragment<FRTSFormationSettings>();
		const FMassMovementParameters& MovementParameters = Context.GetConstSharedFragment<FMassMovementParameters>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
			const FTransform& Transform = TransformFragments[EntityIndex].GetTransform();

			// Create movement action
			MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
			MoveTarget.Forward = (Transform.GetLocation() - MoveTarget.Center).GetSafeNormal();
			MoveTarget.DistanceToGoal = (Transform.GetLocation() - MoveTarget.Center).Length();
			MoveTarget.SlackRadius = 10.f;
			MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
			MoveTarget.DesiredSpeed = FMassInt16Real(MovementParameters.GenerateDesiredSpeed(FormationSettings.RunMovement, Context.GetEntity(EntityIndex).Index));
		}
	});
}

