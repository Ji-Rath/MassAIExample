// Fill out your copyright notice in the Description page of Project Settings.


#include "MassFindResource.h"

#include "MassCommonFragments.h"
#include "MassSmartObjectHandler.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Mass/ResourceEntity.h"
#include "SmartObjectSubsystem.h"

bool FMassFindResource::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MassResourceUserHandle);
	Linker.LinkExternalData(EntityTransformHandle);
	Linker.LinkExternalData(SmartObjectSubsystemHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(ResourceUserHandle);
	return true;
}

EStateTreeRunStatus FMassFindResource::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	FResourceUserFragment& ResourceUserFragment = Context.GetExternalData(ResourceUserHandle);
	FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);
	const FMassSmartObjectHandler MassSmartObjectHandler(
		MassContext.GetMassEntityExecutionContext(),
		SmartObjectSubsystem,
		SignalSubsystem);

	FGameplayTagContainer ResourcesToBuildHouse;
	ResourcesToBuildHouse.AddTag(InstanceData.RockResourceTag);
	ResourcesToBuildHouse.AddTag(InstanceData.WoodResourceTag);

	FGameplayTag NeedTag = FGameplayTag();
	NeedTag = ResourceUserFragment.Tags.HasTag(InstanceData.RockResourceTag) ? NeedTag : InstanceData.RockResourceTag;
	NeedTag = ResourceUserFragment.Tags.HasTag(InstanceData.WoodResourceTag) ? NeedTag : InstanceData.WoodResourceTag;

	if (!NeedTag.IsValid()) { return EStateTreeRunStatus::Failed; } // We dont need an item
	
	FGameplayTagQuery Query = FGameplayTagQuery::MakeQuery_MatchTag(NeedTag);
	InstanceData.RequestID = MassSmartObjectHandler.FindCandidatesAsync(MassContext.GetEntity(), FGameplayTagContainer(), Query, TransformFragment.GetTransform().GetLocation());
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMassFindResource::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	FResourceUserFragment& ResourceUserFragment = Context.GetExternalData(ResourceUserHandle);
	FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);
	const FMassSmartObjectHandler MassSmartObjectHandler(
		MassContext.GetMassEntityExecutionContext(),
		SmartObjectSubsystem,
		SignalSubsystem);

	if (auto CandidateSlots = MassSmartObjectHandler.GetRequestCandidates(InstanceData.RequestID))
	{
		InstanceData.FoundSlots = *CandidateSlots;
		InstanceData.bFoundSmartObject = InstanceData.FoundSlots.NumSlots > 0;

		MassSmartObjectHandler.RemoveRequest(InstanceData.RequestID);
		InstanceData.RequestID.Reset();
		
		SignalSubsystem.SignalEntity(UE::Mass::Signals::LookAtFinished, MassContext.GetEntity());
	}
	return EStateTreeRunStatus::Running;
}

void FMassFindResource::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	FResourceUserFragment& ResourceUserFragment = Context.GetExternalData(ResourceUserHandle);
	FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);
	const FMassSmartObjectHandler MassSmartObjectHandler(
		MassContext.GetMassEntityExecutionContext(),
		SmartObjectSubsystem,
		SignalSubsystem);

	if (InstanceData.RequestID.IsSet())
	{
		MassSmartObjectHandler.RemoveRequest(InstanceData.RequestID);
		InstanceData.RequestID.Reset();
	}
	FMassStateTreeTaskBase::ExitState(Context, Transition);
}

void FMassFindResource::StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus,
	const FStateTreeActiveStates& CompletedActiveStates) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.RequestID.Reset();
	InstanceData.bFoundSmartObject = false;
	
	FMassStateTreeTaskBase::StateCompleted(Context, CompletionStatus, CompletedActiveStates);
}
