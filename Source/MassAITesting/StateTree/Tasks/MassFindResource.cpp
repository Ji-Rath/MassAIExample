// Fill out your copyright notice in the Description page of Project Settings.


#include "MassFindResource.h"

#include "MassCommonFragments.h"
#include "MassSmartObjectHandler.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Mass/ResourceEntity.h"

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
		MassContext.GetEntityManager(),
		MassContext.GetEntitySubsystemExecutionContext(),
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

	FBox AreaBox = FBox(TransformFragment.GetTransform().GetLocation()-10000.f,TransformFragment.GetTransform().GetLocation()+10000.f);
	FSmartObjectRequestFilter Filter(FGameplayTagContainer(),  ESmartObjectClaimPriority::Normal, Query, {}, false, false, false);
	FSmartObjectRequest Request(AreaBox, Filter);
	TArray<FSmartObjectRequestResult> Results;
	SmartObjectSubsystem.FindSmartObjects(Request, Results);

	//InstanceData.RequestID = MassSmartObjectHandler.FindCandidatesAsync(MassContext.GetEntity(), FGameplayTagContainer(), Query, TransformFragment.GetTransform().GetLocation())
	
	Results.Sort([&SmartObjectSubsystem, &TransformFragment](const FSmartObjectRequestResult& Result, const FSmartObjectRequestResult& Result2)
	{
		auto SlotLocation = SmartObjectSubsystem.GetSlotLocation(Result);
		auto SlotLocation2 = SmartObjectSubsystem.GetSlotLocation(Result2);
		if (SlotLocation.IsSet() && SlotLocation2.IsSet())
		{
			return FVector::DistSquared2D(SlotLocation.Get(FVector::ZeroVector), TransformFragment.GetTransform().GetLocation()) < FVector::DistSquared2D(SlotLocation2.Get(FVector::ZeroVector), TransformFragment.GetTransform().GetLocation());
		}
		return false;
	});

	if (!Results.IsEmpty())
	{
		InstanceData.FoundSlots = FMassSmartObjectCandidateSlots({FSmartObjectCandidateSlot(Results[0], 0)}, 1);
		// we dont return succeeded here because that is the responsibility of the other child tasks
	}
	else
	{
		return EStateTreeRunStatus::Failed;
	}
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMassFindResource::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return FMassStateTreeTaskBase::Tick(Context, DeltaTime);
}

void FMassFindResource::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FMassStateTreeTaskBase::ExitState(Context, Transition);
}
