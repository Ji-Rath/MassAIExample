// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletProcessor.h"

#include "BulletFragments.h"
#include "BulletHellSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassSignalSubsystem.h"

UBulletInitializerProcessor::UBulletInitializerProcessor()
	: EntityQuery(*this)
{
}

void UBulletInitializerProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FBulletTag>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FBulletFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UBulletInitializerProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);

	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, BulletHell::Signals::BulletSpawned);
}

void UBulletInitializerProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
                                                 FMassSignalNameLookup& EntitySignals)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		auto SignalSubsystem = Context.GetMutableSubsystem<UMassSignalSubsystem>();
		auto BulletFragments = Context.GetFragmentView<FBulletFragment>();
		auto VelocityFragments = Context.GetMutableFragmentView<FMassVelocityFragment>();
		auto TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& BulletFragment = BulletFragments[EntityIdx];
			auto& VelocityFragment = VelocityFragments[EntityIdx];
			auto& TransformFragment = TransformFragments[EntityIdx];

			VelocityFragment.Value = BulletFragment.Direction.GetSafeNormal() * BulletFragment.Speed;
			TransformFragment.GetMutableTransform().SetLocation(BulletFragment.SpawnLocation);

			SignalSubsystem->DelaySignalEntityDeferred(Context, BulletHell::Signals::BulletDestroy, Context.GetEntity(EntityIdx), BulletFragment.Lifetime);
		}
	});
}

void UBulletDestroyerProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FBulletTag>(EMassFragmentPresence::All);
}

void UBulletDestroyerProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);

	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, BulletHell::Signals::BulletDestroy);
}

void UBulletDestroyerProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			Context.Defer().DestroyEntity(Context.GetEntity(EntityIdx));
		}
	});
}

UBulletDestroyerProcessor::UBulletDestroyerProcessor()
	: EntityQuery(*this)
{
	
}

UBulletCollisionProcessor::UBulletCollisionProcessor()
	: EntityQuery(*this)
{
}

void UBulletCollisionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FBulletTag>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadOnly);
}

void UBulletCollisionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this, &EntityManager](FMassExecutionContext& Context)
	{
		auto BulletHellSubsystem = Context.GetSubsystem<UBulletHellSubsystem>();
		auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& TransformFragment = TransformFragments[EntityIdx];
			auto Location = TransformFragment.GetTransform().GetLocation();
			
			TArray<FMassEntityHandle> Entities;
			BulletHellSubsystem->GetHashGrid().Query(FBox::BuildAABB(Location, FVector(50.f)), Entities);

			Entities = Entities.FilterByPredicate([&Location, &EntityManager](const FMassEntityHandle& Entity)
			{
				auto EntityLocation = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity)->GetTransform().GetLocation();
				return FVector::Dist(Location, EntityLocation) <= 50.f;
			});

			if (Entities.Num() > 0)
			{
				Entities.Add(Context.GetEntity(EntityIdx));
				Context.Defer().DestroyEntities(Entities); // Delete bullet as well
			}
		}
	});
}
