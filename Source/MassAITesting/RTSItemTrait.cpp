// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSItemTrait.h"

#include "RTSBuildingSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassRepresentationFragments.h"
#include "Engine/World.h"

void URTSItemTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	BuildContext.AddFragment<FItemFragment>();
}

UItemProcessor::UItemProcessor()
{
	
}

void UItemProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
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
			BuildingSubsystem->ItemHashGrid.UpdatePoint(Context.GetEntity(EntityIndex), Item.OldLocation, Location);
			Item.OldLocation = Location;

			// Update color of item to represent the item
			// @todo find out if there is a way to initialize the color rather than needlessly update it every frame
			if (Representation.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance)
            {
            	TArray<float> CustomData;
            	CustomData.Reserve(1);
            	
            	CustomData.Emplace(Item.ItemType == Rock ? 0.f : 1.f);
            	
            	MeshInfo[Representation.StaticMeshDescIndex].AddBatchedCustomDataFloats(CustomData, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);

            	Representation.PrevTransform = Transforms[EntityIndex].GetTransform();
            	Representation.PrevLODSignificance = RepresentationLOD.LODSignificance;
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
}

void UItemInitializerProcessor::Initialize(UObject& Owner)
{
	BuildingSubsystem = UWorld::GetSubsystem<URTSBuildingSubsystem>(Owner.GetWorld());
}

void UItemInitializerProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FItemFragment> ItemFragments = Context.GetMutableFragmentView<FItemFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FItemFragment& Item = ItemFragments[EntityIndex];
			FTransform& Transform = Transforms[EntityIndex].GetMutableTransform();
			const FVector& Location = Transform.GetLocation();

			// Add randomness to item spawn location
			Item.OldLocation.X += FMath::FRandRange(-100.f, 100.f);
			Item.OldLocation.Y += FMath::FRandRange(-100.f, 100.f);
			
			// Old location stores the initial spawn location
			BuildingSubsystem->ItemHashGrid.InsertPoint(Context.GetEntity(EntityIndex), Item.OldLocation);
			Transform.SetLocation(Item.OldLocation);
			
			Context.Defer().AddTag<FItemAddedToGrid>(Context.GetEntity(EntityIndex));
		}
	});
}
