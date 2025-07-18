// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentTraits.h"

#include "MassEntitySubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "MassNavigationFragments.h"
#include "MassObserverRegistry.h"
#include "Engine/World.h"

void URTSFormationAgentTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(&World);
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	check(EntitySubsystem);
	
	BuildContext.AddFragment<FRTSFormationAgent>();
	
	auto MySharedFragment = EntityManager.GetOrCreateSharedFragment<FRTSFormationSettings>(FormationSettings);
	BuildContext.AddSharedFragment(MySharedFragment);
}
