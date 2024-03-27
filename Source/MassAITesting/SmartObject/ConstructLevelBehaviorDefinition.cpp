// Fill out your copyright notice in the Description page of Project Settings.


#include "ConstructLevelBehaviorDefinition.h"

#include "MassCommandBuffer.h"
#include "MassSmartObjectFragments.h"
#include "MassAITesting/Mass/RTSConstruction.h"

void UConstructLevelBehaviorDefinition::Activate(FMassCommandBuffer& CommandBuffer,
                                                 const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Activate(CommandBuffer, EntityContext);
	
	FMassSmartObjectUserFragment& SOUser = EntityContext.EntityView.GetFragmentData<FMassSmartObjectUserFragment>();
	//@todo find out how to fix
	//CommandBuffer.PushCommand<FMassCommandAddTag>(EntityContext.EntityView.GetEntity(), FRTSConstructFloor::StaticStruct());
}

void UConstructLevelBehaviorDefinition::Deactivate(FMassCommandBuffer& CommandBuffer,
	const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Deactivate(CommandBuffer, EntityContext);
}
