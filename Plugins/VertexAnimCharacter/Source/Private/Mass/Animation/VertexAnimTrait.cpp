// Fill out your copyright notice in the Description page of Project Settings.


#include "Mass/Animation/VertexAnimTrait.h"

#include "MassEntityTemplateRegistry.h"

void UVertexAnimTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FVertexAnimInfoFragment>();
	BuildContext.AddFragment(FConstStructView::Make(VertexAnimLocomotion));
	
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	const FConstSharedStruct& VertexAnimSharedStruct = EntityManager.GetOrCreateConstSharedFragment(VertexAnimData);
	
	BuildContext.AddConstSharedFragment(VertexAnimSharedStruct);
}
