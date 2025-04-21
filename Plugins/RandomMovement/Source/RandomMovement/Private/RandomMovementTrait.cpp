// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomMovementTrait.h"

#include "MassEntityTemplateRegistry.h"
#include "RandomMovementFragments.h"

void URandomMovementTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	
	BuildContext.AddFragment<FRandomMovementFragment>();

	auto SharedFragment = EntityManager.GetOrCreateConstSharedFragment(RandomMovementSettings);
	BuildContext.AddConstSharedFragment(SharedFragment);
}
