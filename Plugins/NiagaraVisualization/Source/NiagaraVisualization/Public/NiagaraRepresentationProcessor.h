#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"
#include "NiagaraRepresentationProcessor.generated.h"



UCLASS()
class NIAGARAVISUALIZATION_API UNiagaraRepresentationProcessor : public UMassProcessor
{
	GENERATED_BODY()
public:
	
	UNiagaraRepresentationProcessor();
	
	virtual void ConfigureQueries() override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	FMassEntityQuery PositionToNiagaraFragmentQuery;
	FMassEntityQuery NiagaraPositionChunkQuery;
};