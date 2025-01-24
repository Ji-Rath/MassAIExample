// Fill out your copyright notice in the Description page of Project Settings.


#include "BHEnemyFragments.h"

#include "MassEntityTemplateRegistry.h"

void UBHEnemyTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment(FConstStructView::Make(BHEnemyFragment));
	BuildContext.AddTag<FBHEnemyTag>();
}
