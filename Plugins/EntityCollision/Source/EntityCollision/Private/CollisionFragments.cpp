// Fill out your copyright notice in the Description page of Project Settings.


#include "CollisionFragments.h"

#include "MassEntityTemplateRegistry.h"

void UAvoidanceTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FEntityAvoidanceTag>();
	
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	const FConstSharedStruct& AvoidanceSharedFragment = EntityManager.GetOrCreateConstSharedFragment(AvoidanceSettings);
	BuildContext.AddConstSharedFragment(AvoidanceSharedFragment);
}

void UObstacleTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FObstacleTag>();
	BuildContext.AddFragment<FCollisionFragment>();
}
