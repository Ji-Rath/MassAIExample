// Fill out your copyright notice in the Description page of Project Settings.


#include "CollisionFragments.h"

#include "MassEntityTemplateRegistry.h"

void UCollisionTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FCollisionFragment>();
}
