// Fill out your copyright notice in the Description page of Project Settings.


#include "GatherResourceBehaviorDefinition.h"

#include "MassCommandBuffer.h"
#include "MassCommonFragments.h"
#include "MassEntityConfigAsset.h"
#include "MassSmartObjectFragments.h"
#include "MassSpawnerSubsystem.h"
#include "RTSAgentTrait.h"
#include "RTSItemTrait.h"
#include "SmartObjectComponent.h"
#include "Kismet/GameplayStatics.h"

void UGatherResourceBehaviorDefinition::Activate(FMassCommandBuffer& CommandBuffer,
                                                 const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Activate(CommandBuffer, EntityContext);

	// Spawn resource fragment with set values
	FRTSGatherResourceFragment RTSResourceFragment;
	RTSResourceFragment.Resource = ResourceType;
	RTSResourceFragment.Amount = ResourceAmount;
	CommandBuffer.PushCommand(FCommandAddFragmentInstance(EntityContext.EntityView.GetEntity(), FConstStructView::Make(RTSResourceFragment)));

	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(EntityContext.SmartObjectSubsystem.GetWorld());
	UMassSpawnerSubsystem* SpawnerSubsystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(EntityContext.SmartObjectSubsystem.GetWorld());

	// Spawn items on the ground
	// @todo clean up this mess lol
	TArray<FMassEntityHandle> Items;
	const FMassEntityTemplate* EntityTemplate = ItemConfig->GetConfig().GetOrCreateEntityTemplate(*UGameplayStatics::GetPlayerPawn(EntityContext.SmartObjectSubsystem.GetWorld(), 0), *ItemConfig);
	SpawnerSubsystem->SpawnEntities(*EntityTemplate, 5, Items);
	
	for(FMassEntityHandle ItemHandle : Items)
	{
		FTransformFragment* Transform = EntitySubsystem->GetFragmentDataPtr<FTransformFragment>(ItemHandle);
		FItemFragment* Item =  EntitySubsystem->GetFragmentDataPtr<FItemFragment>(ItemHandle);
		if (Transform)
		{
			FVector SpawnLocation = EntityContext.EntityView.GetFragmentDataPtr<FTransformFragment>()->GetTransform().GetLocation();
			SpawnLocation += FVector(FMath::RandRange(-100,100),FMath::RandRange(-100,100),0.f);
			Transform->GetMutableTransform().SetLocation(SpawnLocation);
			Item->ItemType = ResourceType;
		}
	}

	FRTSAgentFragment& Agent = EntityContext.EntityView.GetFragmentData<FRTSAgentFragment>();
	Agent.bPunching = true;
	
	// Traditional way to spawn a default fragment
	//CommandBuffer.AddFragment<FRTSGatherResourceFragment>(EntityContext.EntityView.GetEntity());
}

void UGatherResourceBehaviorDefinition::Deactivate(FMassCommandBuffer& CommandBuffer,
	const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Deactivate(CommandBuffer, EntityContext);
	
	//CommandBuffer.RemoveFragment<FRTSGatherResourceFragment>(EntityContext.EntityView.GetEntity());
	
	FRTSAgentFragment& Agent = EntityContext.EntityView.GetFragmentData<FRTSAgentFragment>();
	Agent.bPunching = false;
	
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
