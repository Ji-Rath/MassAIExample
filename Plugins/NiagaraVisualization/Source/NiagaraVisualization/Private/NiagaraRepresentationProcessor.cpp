// Fill out your copyright notice in the Description page of Project Settings.


#include "NiagaraRepresentationProcessor.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassRepresentationTypes.h"
#include "MassSignalSubsystem.h"
#include "MassSimulationLOD.h"
#include "NiagaraEntityVizActor.h"
#include "MSRepresentationFragments.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

UNiagaraRepresentationProcessor::UNiagaraRepresentationProcessor() :
	NiagaraPositionChunkQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Editor);
	
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Representation;
	ProcessingPhase = EMassProcessingPhase::FrameEnd;
}

void UNiagaraRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	NiagaraPositionChunkQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	NiagaraPositionChunkQuery.AddSharedRequirement<FSharedNiagaraSystemFragment>(EMassFragmentAccess::ReadWrite);
	NiagaraPositionChunkQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
}

// todo-performance separate setup for rarely moving pieces?
void UNiagaraRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	NiagaraPositionChunkQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_PositionChunkQuery);
		const int32 QueryLength = Context.GetNumEntities();

		const auto& Transforms = Context.GetFragmentView<FTransformFragment>();
		auto& SharedNiagaraFragment = Context.GetMutableSharedFragment<FSharedNiagaraSystemFragment>();
		const auto& VelocityFragments = Context.GetFragmentView<FMassVelocityFragment>();
		
		for (int32 i = 0; i < QueryLength; ++i)
		{
			const auto& VelocityFragment = VelocityFragments[i];
			auto& Transform = Transforms[i].GetTransform();
			SharedNiagaraFragment.ParticlePositions.Emplace(Transform.GetTranslation());
			SharedNiagaraFragment.ParticleOrientations.Emplace((FQuat4f)Transform.GetRotation());
			SharedNiagaraFragment.AnimationIndexes.Emplace(VelocityFragment.Value.Length() > 5.f ? 1 : 0); // swtich between idle and running
		}
	});
	
	// Push entity data to the niagara system
	EntityManager.ForEachSharedFragment<FSharedNiagaraSystemFragment>([](FSharedNiagaraSystemFragment& SharedNiagaraFragment)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_MASS_MassToNiagara);
		const ANiagaraEntityVizActor* NiagaraActor = SharedNiagaraFragment.NiagaraManagerActor.Get();

		if (UNiagaraComponent* NiagaraComponent = NiagaraActor->GetNiagaraComponent())
		{

			// congratulations to me (karl) for making SetNiagaraArrayVector public in an engine PR (he's so cool) (wow)
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, SharedNiagaraFragment.ParticlePositionsName,
			                                                                 SharedNiagaraFragment.ParticlePositions);
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayQuat(NiagaraComponent, SharedNiagaraFragment.ParticleOrientationsParameterName,
			                                                               SharedNiagaraFragment.ParticleOrientations);
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayUInt8(NiagaraComponent, SharedNiagaraFragment.AnimationIndexesParameterName,
																		   SharedNiagaraFragment.AnimationIndexes);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("projectile manager %s was invalid during array push!"), *NiagaraActor->GetName());
		}

		// Reset array data so entities can populate info again
		// @todo optimize entities that dont change often
		SharedNiagaraFragment.ParticleOrientations.Reset();
		SharedNiagaraFragment.ParticlePositions.Reset();
		SharedNiagaraFragment.AnimationIndexes.Reset();
	});
}
