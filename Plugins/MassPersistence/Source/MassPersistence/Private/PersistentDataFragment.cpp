// Fill out your copyright notice in the Description page of Project Settings.


#include "PersistentDataFragment.h"

void UPersistentDataTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	
	FConstSharedStruct ParamsFragment = EntityManager.GetOrCreateConstSharedFragment(PersistentDataFragment);
	BuildContext.AddConstSharedFragment(ParamsFragment);

	BuildContext.AddTag<FPersistentDataTag>();
	BuildContext.AddFragment<FPersistentTransformFragment>();
}
