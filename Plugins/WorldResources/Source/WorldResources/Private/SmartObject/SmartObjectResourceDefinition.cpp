// Fill out your copyright notice in the Description page of Project Settings.


#include "SmartObject/SmartObjectResourceDefinition.h"

#include "Mass/ResourceEntity.h"

void USmartObjectResourceDefinition::Activate(FMassCommandBuffer& CommandBuffer,
                                              const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Activate(CommandBuffer, EntityContext);

	auto& ResourceUserFragment = EntityContext.EntityView.GetFragmentData<FResourceUserFragment>();

	if (bRemoveTags)
	{
		ResourceUserFragment.Tags.RemoveTags(ResourceTags);
	}
	else
	{
		ResourceUserFragment.Tags.AppendTags(ResourceTags);
	}
	
}
