#include "NiagaraEntityVizSubsystem.h"

#include "MassEntitySubsystem.h"
#include "NiagaraEntityVizActor.h"
#include "MSRepresentationFragments.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraTickBehaviorEnum.h"

void UNiagaraEntityVizSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	auto MassSubsystem = Collection.InitializeDependency<UMassEntitySubsystem>();
	
	MassManager = MassSubsystem->GetMutableEntityManager().AsShared();
	
}

FSharedStruct UNiagaraEntityVizSubsystem::GetOrCreateSharedNiagaraFragmentForSystemType(UNiagaraSystem* NiagaraSystem, UStaticMesh* StaticMeshOverride, UMaterialInterface* MaterialOverride)
{
	uint32 NiagaraAssetHash = GetTypeHash(NiagaraSystem->GetPathName());
	uint32 ParamsHash = NiagaraAssetHash;
	if(StaticMeshOverride)
	{
		ParamsHash = HashCombineFast(NiagaraAssetHash,GetTypeHash(StaticMeshOverride->GetFName()));
	}
	if(MaterialOverride)
	{
		ParamsHash = HashCombineFast(NiagaraAssetHash,GetTypeHash(MaterialOverride->GetFName()));
	}
	FSharedNiagaraSystemFragment SharedStructToReturn = FSharedNiagaraSystemFragment();

	// Grab the existing shared fragment if already created
	if(PreexistingSharedNiagaraActors.Contains(ParamsHash))
	{
		return MassManager->GetOrCreateSharedFragment<FSharedNiagaraSystemFragment>(SharedStructToReturn);
	}

	FActorSpawnParameters SpawnParameters;

	SpawnParameters.ObjectFlags = RF_Transient | RF_DuplicateTransient;

	// Spawn niagara actor for visualizing entities
	ANiagaraEntityVizActor* NewNiagaraActor = GetWorld()->SpawnActor<ANiagaraEntityVizActor>(SpawnParameters);

	// We need this to tick last so that it receives the new gameplay state we create in the mass manager (stuff moving etc) for the next frame.
	NewNiagaraActor->GetNiagaraComponent()->SetTickBehavior(ENiagaraTickBehavior::ForceTickLast);
	NewNiagaraActor->GetNiagaraComponent()->SetAsset(NiagaraSystem);

	if(StaticMeshOverride)
	{
		NewNiagaraActor->GetNiagaraComponent()->SetVariableStaticMesh("StaticMeshToRender", StaticMeshOverride);

		if(MaterialOverride)
		{
			NewNiagaraActor->GetNiagaraComponent()->SetVariableMaterial("StaticMeshMaterial", MaterialOverride);
		}
		else
		{
			NewNiagaraActor->GetNiagaraComponent()->SetVariableMaterial("StaticMeshMaterial", StaticMeshOverride->GetMaterial(0));
		}
	}
	SharedStructToReturn.NiagaraManagerActor = NewNiagaraActor;

	PreexistingSharedNiagaraActors.FindOrAdd(ParamsHash,NewNiagaraActor);
	
	return MassManager->GetOrCreateSharedFragment<FSharedNiagaraSystemFragment>(SharedStructToReturn);
}

