// Fill out your copyright notice in the Description page of Project Settings.


#include "ConstructLevelBehaviorDefinition.h"

#include "MassSmartObjectFragments.h"
#include "RTSAgentTrait.h"

void UConstructLevelBehaviorDefinition::Activate(FMassCommandBuffer& CommandBuffer,
                                                 const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Activate(CommandBuffer, EntityContext);
	// todo: check GetWorld() before accessing
	FRTSBuildingFragment RTSBuildingFragment;
	const UMassEntitySubsystem* EntitySubsystem = EntityContext.SmartObjectSubsystem.GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	const FMassSmartObjectUserFragment SOUser = EntitySubsystem->GetFragmentDataChecked<FMassSmartObjectUserFragment>(EntityContext.EntityView.GetEntity());
	RTSBuildingFragment.BuildingClaimHandle = SOUser.ClaimHandle;
	CommandBuffer.PushCommand(FCommandAddFragmentInstance(EntityContext.EntityView.GetEntity(), FConstStructView::Make(RTSBuildingFragment)));

	CommandBuffer.AddTag<FRTSRequestResources>(EntityContext.EntityView.GetEntity());
}

void UConstructLevelBehaviorDefinition::Deactivate(FMassCommandBuffer& CommandBuffer,
	const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Deactivate(CommandBuffer, EntityContext);
}
