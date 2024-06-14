// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSItemTrait.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassRepresentationFragments.h"
#include "Engine/World.h"
#include "MassAITesting/RTSBuildingSubsystem.h"

void URTSItemTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FItemFragment>();
}

UItemProcessor::UItemProcessor()
{
	
}

void UItemProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		TArrayView<FItemFragment> ItemFragments = Context.GetMutableFragmentView<FItemFragment>();
		TArrayView<FMassRepresentationFragment> RepresentationFragments = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		TConstArrayView<FMassRepresentationLODFragment> RepresentationLODFragments = Context.GetFragmentView<FMassRepresentationLODFragment>();

		FMassInstancedStaticMeshInfoArrayView MeshInfo = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FItemFragment& Item = ItemFragments[EntityIndex];
			const FVector& Location = Transforms[EntityIndex].GetTransform().GetLocation();
			FMassRepresentationFragment& Representation = RepresentationFragments[EntityIndex];
			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODFragments[EntityIndex];

			// Update Item Hash Grid with new position (Probably does not need to be done for an item...that should keep the same location)
			float Radius = 25.f;
			const FBox OldBounds(Item.OldLocation - FVector(Radius, Radius, 0.f), Item.OldLocation + FVector(Radius, Radius, 0.f));
			const FBox NewBounds(Location - FVector(Radius, Radius, 0.f), Location + FVector(Radius, Radius, 0.f));
			
			Item.CellLoc = BuildingSubsystem->ItemHashGrid.Move(Context.GetEntity(EntityIndex), OldBounds, NewBounds);
			Item.OldLocation = Location;

			//@todo move this to its own processor
			if (MeshInfo.IsValidIndex(Representation.StaticMeshDescHandle.ToIndex()))
			{
				MeshInfo[Representation.StaticMeshDescHandle.ToIndex()].AddBatchedCustomData<float>(Item.ItemType == Rock ? 0.f : 1.f, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
			}
		}
	});
}

void UItemProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FItemFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FItemAddedToGrid>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.SetChunkFilter(&FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk);
	EntityQuery.RegisterWithProcessor(*this);
}

void UItemProcessor::Initialize(UObject& Owner)
{
	BuildingSubsystem = UWorld::GetSubsystem<URTSBuildingSubsystem>(Owner.GetWorld());

	RepresentationSubsystem = UWorld::GetSubsystem<UMassRepresentationSubsystem>(Owner.GetWorld());
}

UItemInitializerProcessor::UItemInitializerProcessor()
{
	ObservedType = FItemFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UItemInitializerProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FItemFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UItemInitializerProcessor::Initialize(UObject& Owner)
{
	BuildingSubsystem = UWorld::GetSubsystem<URTSBuildingSubsystem>(Owner.GetWorld());
	RepresentationSubsystem = UWorld::GetSubsystem<UMassRepresentationSubsystem>(Owner.GetWorld());
}

void UItemInitializerProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FItemFragment> ItemFragments = Context.GetMutableFragmentView<FItemFragment>();

		FMassInstancedStaticMeshInfoArrayView MeshInfo = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FItemFragment& Item = ItemFragments[EntityIndex];
			FTransform& Transform = Transforms[EntityIndex].GetMutableTransform();

			if (Transform.GetLocation() == FVector::Zero())
			{
				// Add randomness to item spawn location
				Item.OldLocation.X += FMath::FRandRange(-100.f, 100.f);
				Item.OldLocation.Y += FMath::FRandRange(-100.f, 100.f);
				Transform.SetLocation(Item.OldLocation);
			}
			// Old location stores the initial spawn location
			float Radius = 25.f;
			const FBox NewBounds(Item.OldLocation - FVector(Radius, Radius, 0.f), Item.OldLocation + FVector(Radius, Radius, 0.f));
			Item.CellLoc = BuildingSubsystem->ItemHashGrid.Add(Context.GetEntity(EntityIndex), NewBounds);
			
			Context.Defer().AddTag<FItemAddedToGrid>(Context.GetEntity(EntityIndex));
		}
	});
}
