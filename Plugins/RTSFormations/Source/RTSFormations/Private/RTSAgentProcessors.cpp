// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentProcessors.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "RTSAgentTraits.h"
#include "Engine/World.h"

//----------------------------------------------------------------------//
//  URTSUpdateHashPosition
//----------------------------------------------------------------------//
void URTSUpdateHashPosition::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRTSCellLocFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FRTSAgentHashTag>(EMassFragmentPresence::All);
	EntityQuery.AddSubsystemRequirement<URTSAgentSubsystem>(EMassFragmentAccess::ReadWrite);
}

URTSUpdateHashPosition::URTSUpdateHashPosition()
	: EntityQuery(*this)
{
}

void URTSUpdateHashPosition::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		auto CellLocFragments = Context.GetMutableFragmentView<FRTSCellLocFragment>();
		TConstArrayView<FAgentRadiusFragment> RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
		auto& AgentSubsystem = Context.GetMutableSubsystemChecked<URTSAgentSubsystem>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			auto& CellLocFragment = CellLocFragments[EntityIndex];
			const FVector& Location = TransformFragments[EntityIndex].GetTransform().GetLocation();
			const float Radius = RadiusFragments[EntityIndex].Radius;
			
			const FBox NewBounds(Location - FVector(Radius, Radius, 0.f), Location + FVector(Radius, Radius, 0.f));
			CellLocFragment.CellLoc = AgentSubsystem.AgentHashGrid.Move(Context.GetEntity(EntityIndex), CellLocFragment.CellLoc, NewBounds);
		}
	});
}

//----------------------------------------------------------------------//
//  URTSInitializeHashPosition
//----------------------------------------------------------------------//
URTSInitializeHashPosition::URTSInitializeHashPosition()
	: EntityQuery(*this)
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void URTSInitializeHashPosition::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRTSCellLocFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FRTSAgentHashTag>(EMassFragmentPresence::None);
	EntityQuery.AddSubsystemRequirement<URTSAgentSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void URTSInitializeHashPosition::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		auto CellLocFragments = Context.GetMutableFragmentView<FRTSCellLocFragment>();
		TConstArrayView<FAgentRadiusFragment> RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
		auto& AgentSubsystem = Context.GetMutableSubsystemChecked<URTSAgentSubsystem>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			auto& CellLocFragment = CellLocFragments[EntityIndex];
			const FVector& Location = TransformFragments[EntityIndex].GetTransform().GetLocation();
			const float Radius = RadiusFragments[EntityIndex].Radius;
			
			const FBox NewBounds(Location - FVector(Radius, Radius, 0.f), Location + FVector(Radius, Radius, 0.f));
			UE_LOG(LogTemp, Log, TEXT("Agents: %d"), AgentSubsystem.AgentHashGrid.GetItems().Num());
			CellLocFragment.CellLoc = AgentSubsystem.AgentHashGrid.Add(Context.GetEntity(EntityIndex), NewBounds);
			
			Context.Defer().AddTag<FRTSAgentHashTag>(Context.GetEntity(EntityIndex));
		}
	});
}

//----------------------------------------------------------------------//
//  URTSRemoveHashPosition
//----------------------------------------------------------------------//

URTSRemoveHashPosition::URTSRemoveHashPosition()
	: EntityQuery(*this)
{
	ObservedType = FRTSFormationAgent::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

void URTSRemoveHashPosition::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRTSCellLocFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URTSAgentSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void URTSRemoveHashPosition::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		auto CellLocFragments = Context.GetFragmentView<FRTSCellLocFragment>();
		auto& AgentSubsystem = Context.GetMutableSubsystemChecked<URTSAgentSubsystem>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const auto& RTSAgent = CellLocFragments[EntityIndex];
			
			AgentSubsystem.AgentHashGrid.Remove(Context.GetEntity(EntityIndex), RTSAgent.CellLoc);
		}
	});
}