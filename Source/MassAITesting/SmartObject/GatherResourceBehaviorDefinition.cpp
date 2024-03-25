// Fill out your copyright notice in the Description page of Project Settings.


#include "GatherResourceBehaviorDefinition.h"

#include "MassCommandBuffer.h"
#include "MassCommonFragments.h"
#include "MassEntityConfigAsset.h"
#include "MassSmartObjectFragments.h"
#include "MassSpawnerSubsystem.h"
#include "SmartObjectComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MassAITesting/Mass/RTSItemTrait.h"

void UGatherResourceBehaviorDefinition::Activate(FMassCommandBuffer& CommandBuffer,
                                                 const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Activate(CommandBuffer, EntityContext);

	// Spawn resource fragment with set values
	/*
	FRTSGatherResourceFragment RTSResourceFragment;
	RTSResourceFragment.Resource = ResourceType;
	RTSResourceFragment.Amount = ResourceAmount;
	CommandBuffer.PushCommand(FCommandAddFragmentInstance(EntityContext.EntityView.GetEntity(), FConstStructView::Make(RTSResourceFragment)));
	*/

	FRTSAgentFragment& Agent = EntityContext.EntityView.GetFragmentData<FRTSAgentFragment>();
	Agent.bPunching = true;

	// Invalidate resource handle when complete
	Agent.ResourceHandle.Invalidate();
	
	// Traditional way to spawn a default fragment
	//CommandBuffer.AddFragment<FRTSGatherResourceFragment>(EntityContext.EntityView.GetEntity());
}

void UGatherResourceBehaviorDefinition::Deactivate(FMassCommandBuffer& CommandBuffer,
	const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Deactivate(CommandBuffer, EntityContext);
	
	//CommandBuffer.RemoveFragment<FRTSGatherResourceFragment>(EntityContext.EntityView.GetEntity());

	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(EntityContext.SmartObjectSubsystem.GetWorld());
	UMassSpawnerSubsystem* SpawnerSubsystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(EntityContext.SmartObjectSubsystem.GetWorld());

	if (EntityContext.SmartObjectSubsystem.GetWorld() && UGameplayStatics::GetPlayerPawn(EntityContext.SmartObjectSubsystem.GetWorld(), 0))
	{
		// Spawn items on the ground
		// @todo clean up this mess lol
		TArray<FMassEntityHandle> Items;
		const FMassEntityTemplate EntityTemplate = ItemConfig->GetConfig().GetOrCreateEntityTemplate(*EntityContext.SmartObjectSubsystem.GetWorld());
		SpawnerSubsystem->SpawnEntities(EntityTemplate, 4, Items);
	
		for(const FMassEntityHandle& ItemHandle : Items)
		{
			const FVector& SpawnLocation = EntityContext.EntityView.GetFragmentDataPtr<FTransformFragment>()->GetTransform().GetLocation();
			
			FItemFragment ItemFragment;
			ItemFragment.ItemType = ResourceType;
			ItemFragment.OldLocation = SpawnLocation;
			CommandBuffer.PushCommand<FMassCommandAddFragmentInstances>(ItemHandle, FConstStructView::Make(ItemFragment));
		}
	
		FRTSAgentFragment& Agent = EntityContext.EntityView.GetFragmentData<FRTSAgentFragment>();
		Agent.bPunching = false;
	
		const FMassSmartObjectUserFragment& SOUser = EntityContext.EntityView.GetFragmentData<FMassSmartObjectUserFragment>();
		if (USmartObjectComponent* SOComp = EntityContext.SmartObjectSubsystem.GetSmartObjectComponent(SOUser.InteractionHandle))
		{
			CommandBuffer.PushCommand<FMassDeferredAddCommand>([SOComp, EntityContext](UMassEntitySubsystem& System)
			{
				SOComp->GetOwner()->Destroy();
			});
		}
	}
}
