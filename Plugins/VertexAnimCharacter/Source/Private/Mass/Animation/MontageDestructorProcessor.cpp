﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "MontageDestructorProcessor.h"

#include "MassActorSubsystem.h"
#include "MassRepresentationFragments.h"
#include "Mass/Animation/VertexAnimProcessor.h"

UMontageDestructorProcessor::UMontageDestructorProcessor()
	: EntityQuery(*this)
{
	ObservedType = FMassMontageFragment::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

void UMontageDestructorProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadOnly);
}

void UMontageDestructorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const auto& ActorFragments = Context.GetFragmentView<FMassActorFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			const auto& ActorFragment = ActorFragments[EntityIdx];
			if (auto Actor = ActorFragment.Get())
			{
				auto SKM = Actor->FindComponentByClass<USkeletalMeshComponent>();
				if (!SKM) { continue; }
				auto AnimInstance = SKM->GetAnimInstance();
				if (!AnimInstance) { continue; }
				AnimInstance->Montage_Stop(0.5f, nullptr); // hacky fix
			}
		}
	});
}
