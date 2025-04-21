#pragma once
#include "CoreMinimal.h"
#include "MassArchetypeTypes.h"
#include "NiagaraEntityVizActor.h"
#include "NiagaraEntityVizSubsystem.generated.h"

UCLASS()
class NIAGARAVISUALIZATION_API UNiagaraEntityVizSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
protected:
	
	TSharedPtr<FMassEntityManager> MassManager;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override
	{
		MassManager.Reset();
		PreexistingSharedNiagaraActors.Empty();
	};

public:
	FSharedStruct GetOrCreateSharedNiagaraFragmentForSystemType(class UNiagaraSystem* NiagaraSystem, UStaticMesh* StaticMeshOverride, UMaterialInterface* MaterialOverride = nullptr);
	

	UPROPERTY()
	TMap<uint32, ANiagaraEntityVizActor*> PreexistingSharedNiagaraActors;
};
