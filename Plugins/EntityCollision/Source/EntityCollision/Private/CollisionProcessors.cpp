// Fill out your copyright notice in the Description page of Project Settings.


#include "CollisionProcessors.h"

#include "CollisionFragments.h"
#include "CollisionSubsystem.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassLODFragments.h"
#include "MassMovementFragments.h"

static float HalfRange = 25.f;

UCollisionInitializerProcessor::UCollisionInitializerProcessor() :
	EntityQuery(*this)
{
	ObservedType = FCollisionFragment::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Add;
}

void UCollisionInitializerProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UCollisionSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UCollisionInitializerProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		auto& HashGridSubsystem = Context.GetMutableSubsystemChecked<UCollisionSubsystem>();
		
		const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const auto HashGridFragments = Context.GetMutableFragmentView<FCollisionFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& HashGridFragment = HashGridFragments[EntityIdx];
			auto& TransformFragment = TransformFragments[EntityIdx];
			auto Location = TransformFragment.GetTransform().GetLocation();
			
			FBox Bounds = { Location - HalfRange, Location + HalfRange };
			HashGridFragment.CellLocation = HashGridSubsystem.HashGridData.Add(Context.GetEntity(EntityIdx), Bounds);
		}
	});
}

UCollisionDestroyProcessor::UCollisionDestroyProcessor() :
	EntityQuery(*this)
{
	ObservedType = FCollisionFragment::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Remove;
}

void UCollisionDestroyProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UCollisionSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UCollisionDestroyProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		auto& HashGridSubsystem = Context.GetMutableSubsystemChecked<UCollisionSubsystem>();
		
		const auto HashGridFragments = Context.GetFragmentView<FCollisionFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& HashGridFragment = HashGridFragments[EntityIdx];

			HashGridSubsystem.HashGridData.Remove(Context.GetEntity(EntityIdx), HashGridFragment.CellLocation);
		}
	});
}

UCollisionProcessor::UCollisionProcessor() :
	EntityQuery(*this),
	CollisionQuery(*this)
{
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Movement);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Avoidance;
}

void UCollisionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UCollisionSubsystem>(EMassFragmentAccess::ReadWrite);
	
	CollisionQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	CollisionQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	CollisionQuery.AddSubsystemRequirement<UCollisionSubsystem>(EMassFragmentAccess::ReadOnly);
	CollisionQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	CollisionQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::None); // used for filtering entities
	CollisionQuery.AddTagRequirement<FMassOffLODTag>(EMassFragmentPresence::None);
}

void UCollisionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Update hash grid position
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UpdateCollisionHashGrid)
		auto& HashGridSubsystem = Context.GetMutableSubsystemChecked<UCollisionSubsystem>();
		
		const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const auto HashGridFragments = Context.GetMutableFragmentView<FCollisionFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& HashGridFragment = HashGridFragments[EntityIdx];
			auto& TransformFragment = TransformFragments[EntityIdx];
			const auto& Location = TransformFragment.GetTransform().GetLocation();

			// Move the entity to the new location
			FBox Bounds = { Location - HalfRange, Location + HalfRange };
			auto NewCellLocation = HashGridSubsystem.HashGridData.Move(Context.GetEntity(EntityIdx), HashGridFragment.CellLocation, Bounds);
			HashGridFragment.CellLocation = NewCellLocation;
		}
	});

	// If we are in range of an entity, push out of it
	CollisionQuery.ParallelForEachEntityChunk(Context, [this, &EntityManager](FMassExecutionContext& Context)
	{
		const auto& HashGridSubsystem = Context.GetSubsystemChecked<UCollisionSubsystem>();
		
		const auto TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();
		const auto RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
		const auto VelocityFragments = Context.GetMutableFragmentView<FMassVelocityFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& TransformFragment = TransformFragments[EntityIdx];
			const auto& RadiusFragment = RadiusFragments[EntityIdx];
			auto& Velocity = VelocityFragments[EntityIdx];
			
			const auto Radius = RadiusFragment.Radius;
			auto& Transform = TransformFragment.GetMutableTransform();
			
			FBox Bounds = { Transform.GetLocation() - HalfRange/2, Transform.GetLocation() + HalfRange/2 };
			
			TArray<FMassEntityHandle> Entities;
			HashGridSubsystem.HashGridData.QuerySmall(Bounds, Entities);

			// Ignore ourselves
			Entities = Entities.FilterByPredicate([&Context, EntityIdx](const FMassEntityHandle& OtherEntity)
			{
				return OtherEntity != Context.GetEntity(EntityIdx);
			});
			
			FVector HitNormal = ResolveCollisions(Entities, EntityManager, Radius, Transform);
			Velocity.Value = FVector::VectorPlaneProject(Velocity.Value, HitNormal);
		}
	});
}

FVector UCollisionProcessor::ResolveCollisions(const TArray<FMassEntityHandle>& Entities,
                                               FMassEntityManager& EntityManager, float Radius,
                                               FTransform& EntityTransform)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CalculateCollision)
	FVector HitNormal(FVector::ZeroVector);
	for (auto& Entity : Entities)
	{
		auto OtherEntityTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
		auto OtherLocation = OtherEntityTransform->GetTransform().GetLocation();
		auto DistSq = FVector::DistSquared(EntityTransform.GetLocation(), OtherLocation);

		// Entities are overlapping, move entity
		if (DistSq < FMath::Square(Radius*2))
		{
			auto Direction = (EntityTransform.GetLocation() - OtherLocation).GetSafeNormal();
			Direction.Z = 0.f;

			auto Radii = Radius*2;
			auto Depth = Radii - FMath::Sqrt(DistSq) + 0.01f;
			
			EntityTransform.SetLocation(EntityTransform.GetLocation() + Depth/2*Direction);
			HitNormal = Direction;
		}
	}
	
	return HitNormal;
}
