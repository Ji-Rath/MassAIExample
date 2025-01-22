// Fill out your copyright notice in the Description page of Project Settings.


#include "MassBuildHouseTask.h"

#include "MassStateTreeExecutionContext.h"
#include "ResourceTags.h"
#include "StateTreeLinker.h"
#include "Mass/ResourceEntity.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_ResourceTree, "Object.Tree");
UE_DEFINE_GAMEPLAY_TAG(TAG_ResourceRock, "Object.Rock");

bool FMassBuildHouseTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ResourceUserHandle);
	return true;
}

EStateTreeRunStatus FMassBuildHouseTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	auto& ResourceUserFragment = Context.GetExternalData(ResourceUserHandle);

	// Ensure we have the resources
	TArray<FGameplayTag, TInlineAllocator<2>> Resources = {TAG_ResourceRock, TAG_ResourceTree};
	auto ResourcesToBuildHouse = FGameplayTagContainer::CreateFromArray(Resources);
	if (!ResourceUserFragment.Tags.HasAll(ResourcesToBuildHouse)) { return EStateTreeRunStatus::Failed; }

	ResourceUserFragment.Tags.Reset();

	FTransform SpawnTransform(InstanceData.Location);
	Context.GetWorld()->SpawnActor(InstanceData.House.LoadSynchronous(), &SpawnTransform);
	
	return EStateTreeRunStatus::Succeeded;
}
