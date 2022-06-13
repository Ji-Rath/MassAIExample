// Fill out your copyright notice in the Description page of Project Settings.


#include "GatherResourceBehaviorDefinition.h"

#include "MassCommandBuffer.h"
#include "MassSmartObjectFragments.h"
#include "RTSAgentTrait.h"
#include "SmartObjectComponent.h"

void UGatherResourceBehaviorDefinition::Activate(FMassCommandBuffer& CommandBuffer,
                                                  const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Activate(CommandBuffer, EntityContext);

	// Spawn resource fragment with set values
	FRTSGatherResourceFragment RTSResourceFragment;
	RTSResourceFragment.Resource = ResourceType;
	RTSResourceFragment.Amount = ResourceAmount;
	CommandBuffer.PushCommand(FCommandAddFragmentInstance(EntityContext.EntityView.GetEntity(), FConstStructView::Make(RTSResourceFragment)));
	
	// Traditional way to spawn a default fragment
	//CommandBuffer.AddFragment<FRTSGatherResourceFragment>(EntityContext.EntityView.GetEntity());
}

void UGatherResourceBehaviorDefinition::Deactivate(FMassCommandBuffer& CommandBuffer,
	const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Deactivate(CommandBuffer, EntityContext);
	
	//CommandBuffer.RemoveFragment<FRTSGatherResourceFragment>(EntityContext.EntityView.GetEntity());
	
	const FMassSmartObjectUserFragment& SOUser = EntityContext.EntityView.GetFragmentData<FMassSmartObjectUserFragment>();
	if (USmartObjectComponent* SOComp = EntityContext.SmartObjectSubsystem.GetSmartObjectComponent(SOUser.ClaimHandle))
	{
		CommandBuffer.PushCommand(FDeferredCommand([SOComp, EntityContext](UMassEntitySubsystem& System)
		{
			EntityContext.SmartObjectSubsystem.UnregisterSmartObject(*SOComp);
			SOComp->GetOwner()->Destroy();
		}));
	}
}
