// Fill out your copyright notice in the Description page of Project Settings.
#include "MSNiagaraRepresentationTraits.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "NiagaraEntityVizSubsystem.h"
#include "MSRepresentationFragments.h"

void UMSNiagaraRepresentationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	// Evil main thread loads
	StaticMesh.LoadSynchronous();
	SharedNiagaraSystem.LoadSynchronous();
	MaterialOverride.LoadSynchronous();
	
	UNiagaraEntityVizSubsystem* NiagaraSubsystem = UWorld::GetSubsystem<UNiagaraEntityVizSubsystem>(&World);

	BuildContext.RequireFragment<FTransformFragment>();

	UMaterial* Material = nullptr;

	if(MaterialOverride)
	{
	}
	
	if (!BuildContext.IsInspectingData())
	{
		FSharedStruct SharedFragment = NiagaraSubsystem->GetOrCreateSharedNiagaraFragmentForSystemType(SharedNiagaraSystem.Get(),StaticMesh.Get(),MaterialOverride.Get());
		BuildContext.AddSharedFragment(SharedFragment);
	}
	else
	{
		FSharedStruct SharedFragment = EntityManager.GetOrCreateSharedFragment<FSharedNiagaraSystemFragment>();
		BuildContext.AddSharedFragment(SharedFragment);
	}
	
}
