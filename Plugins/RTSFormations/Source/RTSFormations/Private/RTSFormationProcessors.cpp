#include "RTSFormationProcessors.h"

#include "LaunchEntityProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavigationTypes.h"
#include "MassSignalSubsystem.h"
#include "MassSimulationLOD.h"
#include "RTSAgentTraits.h"
#include "RTSFormationSubsystem.h"
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
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URTSFormationSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URTSFormationInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// First query is to give all units an appropriate unit index.
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		TArrayView<FRTSFormationAgent> RTSFormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();
		auto& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>();
		auto& FormationSubsystem = Context.GetMutableSubsystemChecked<URTSFormationSubsystem>();

		// Signal affected units/entities at the end
		TArray<FMassEntityHandle> UnitSignals;
		UnitSignals.Reserve(FormationSubsystem.Units.Num());

		// Since we can have multiple units, reserving is only done in the formation subsystem
		// This is because it might be possible that a batch of spawned entities should go to different units
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FRTSFormationAgent& RTSFormationAgent = RTSFormationAgents[EntityIndex];
			auto& UnitFragment = Context.GetEntityManagerChecked().GetFragmentDataChecked<FUnitFragment>(RTSFormationAgent.UnitHandle);
			
			RTSFormationAgent.EntityIndex = UnitFragment.Entities.Num();
			UnitFragment.Entities.Emplace(Context.GetEntity(EntityIndex));

			UnitSignals.AddUnique(RTSFormationAgent.UnitHandle);
		}
		// Signal entities in the unit that their position is updated
		// @todo only notify affected entities
		for(const auto& Unit : UnitSignals)
		{
			auto& UnitFragment = Context.GetEntityManagerChecked().GetFragmentDataChecked<FUnitFragment>(Unit);
			FormationSubsystem.SetUnitPosition(UnitFragment.UnitPosition, Unit);
		}
	});
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
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URTSFormationSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URTSFormationDestroyer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		auto& FormationSubsystem = Context.GetMutableSubsystemChecked<URTSFormationSubsystem>();
		TConstArrayView<FRTSFormationAgent> FormationAgents = Context.GetFragmentView<FRTSFormationAgent>();

		// Signal affected units/entities at the end
		TArray<FMassEntityHandle> UnitSignals;
		UnitSignals.Reserve(FormationSubsystem.Units.Num());
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FRTSFormationAgent& FormationAgent = FormationAgents[EntityIndex];
			auto& UnitFragment = Context.GetEntityManagerChecked().GetFragmentDataChecked<FUnitFragment>(FormationAgent.UnitHandle);
			
			// Remove entity from units array
			if (true)
			{
				const FMassEntityHandle* ItemIndex = UnitFragment.Entities.Find(Context.GetEntity(EntityIndex));
				if (ItemIndex)
				{
					// Since we are caching the index, we need to fix the entity index that replaces the destroyed one
					// Not sure if this is the 'correct' way to handle this, but it works for now
					
					UnitFragment.Entities.Remove(*ItemIndex);
					UnitSignals.AddUnique(FormationAgent.UnitHandle);
				}
			}
		}

		// Signal affected units/entities
		for(const auto& Unit : UnitSignals)
		{
			auto& UnitFragment = Context.GetEntityManagerChecked().GetFragmentDataChecked<FUnitFragment>(Unit);
			if (true)
			{
				//@todo add a consistent way to reference units since the index isn't reliable
				if (UnitFragment.Entities.Num() == 0)
				{
					FormationSubsystem.Units.RemoveSwap(Unit);
					continue;
				}

				// Really the only time we should notify every entity in the unit is when the center point changes
				// Every other time we just have to notify the entity that is replacing the destroyed one
				UnitFragment.Entities.Shrink();
				FormationSubsystem.UpdateUnitPosition(UnitFragment.UnitPosition, Unit);
			}
		}
	});
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
	EntityQuery.AddSharedRequirement<FRTSFormationSettings>(EMassFragmentAccess::ReadOnly);
	
	EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.SetChunkFilter(&FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);

	EntityQuery.AddSubsystemRequirement<URTSFormationSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void URTSAgentMovement::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		TConstArrayView<FRTSFormationAgent> RTSFormationAgents = Context.GetFragmentView<FRTSFormationAgent>();

		const FRTSFormationSettings& FormationSettings = Context.GetSharedFragment<FRTSFormationSettings>();
		const FMassMovementParameters& MovementParameters = Context.GetConstSharedFragment<FMassMovementParameters>();

		auto& FormationSubsystem = Context.GetMutableSubsystemChecked<URTSFormationSubsystem>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
			const FTransform& Transform = TransformFragments[EntityIndex].GetTransform();
			
			const FRTSFormationAgent& RTSFormationAgent = RTSFormationAgents[EntityIndex];

			auto& UnitFragment = Context.GetEntityManagerChecked().GetFragmentDataChecked<FUnitFragment>(RTSFormationAgent.UnitHandle);

			if(MoveTarget.GetCurrentAction() == EMassMovementAction::Stand)
				MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());

			FVector Offset = RTSFormationAgent.Offset;
			if (UnitFragment.bBlendAngle)
			{
				Offset = Offset.RotateAngleAxis(UnitFragment.OldRotation.Yaw, FVector(0.f,0.f,-1.f));
				Offset = Offset.RotateAngleAxis(UnitFragment.InterpRotation.Yaw, FVector(0.f,0.f,1.f));
			}
			MoveTarget.Center = UnitFragment.InterpolatedDestination + Offset;
			
			// Update move target values
			MoveTarget.DistanceToGoal = (MoveTarget.Center - Transform.GetLocation()).Length();
			MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();
			
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
	SubscribeToSignal(*SignalSubsystem, FormationUpdated);
}

void URTSFormationUpdate::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	EntityQuery.AddSharedRequirement<FRTSFormationSettings>(EMassFragmentAccess::ReadOnly);
}

void URTSFormationUpdate::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	// Query to calculate move target for entities based on unit index
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();

		const FRTSFormationSettings& FormationSettings = Context.GetSharedFragment<FRTSFormationSettings>();
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

//----------------------------------------------------------------------//
//  URTSUpdateEntityIndex
//----------------------------------------------------------------------//
void URTSUpdateEntityIndex::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	auto SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, UpdateIndex);
}

void URTSUpdateEntityIndex::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URTSFormationSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URTSUpdateEntityIndex::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	// Update entity index so that they go to the closest possible position
	// Entities are signaled in order of distance to destination, this allows the NewPosition array to be sorted once
	// and cut down on iterations significantly
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		auto& FormationSubsystem = Context.GetMutableSubsystemChecked<URTSFormationSubsystem>();
		TArrayView<FRTSFormationAgent> FormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FRTSFormationAgent& FormationAgent = FormationAgents[EntityIndex];
			auto& UnitFragment = Context.GetEntityManagerChecked().GetFragmentDataChecked<FUnitFragment>(FormationAgent.UnitHandle);
			
			// Get first index since it is sorted
			TPair<int, FVector> ClosestPos;
			float ClosestDistance = -1;
			int i=0;
			
			for(const TPair<int, FVector>& NewPos : UnitFragment.NewPositions)
			{
				float Dist = FVector::DistSquared2D(NewPos.Value, FormationAgent.Offset);
				if (ClosestDistance == -1 || Dist < ClosestDistance)
				{
					ClosestPos = NewPos;
					ClosestDistance = Dist;
				
					// While its not perfect, this adds a hard cap to how many positions to check
					//if (++i > FormationSubsystem->Units[FormationAgent.UnitIndex].FormationLength*2)
					//	break;
				}
			}

			// Basically scoot up entities if there is space in the front
			int& Index = ClosestPos.Key;

			{
				SCOPED_NAMED_EVENT(STAT_RTS_RemoveClaimedPosition, FColor::Green);
				FormationAgent.EntityIndex = Index;
				FormationAgent.Offset = ClosestPos.Value;
				UnitFragment.NewPositions.Remove(Index);
			}

			// Call subsystem function to get entities to move
			if (UnitFragment.NewPositions.Num() == 0)
			{
				FormationSubsystem.MoveEntities(FormationAgent.UnitHandle);
			}
		}
	});
}


