// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSItemTrait.h"

#include "RTSBuildingSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"

void URTSItemTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	BuildContext.AddFragment<FItemFragment>();
}

UItemInitializerProcessor::UItemInitializerProcessor()
{
	ObservedType = FItemFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UItemInitializerProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FItemFragment>(EMassFragmentAccess::None);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UItemInitializerProcessor::Initialize(UObject& Owner)
{
	BuildingSubsystem = UWorld::GetSubsystem<URTSBuildingSubsystem>(Owner.GetWorld());
}

UItemRemoverProcessor::UItemRemoverProcessor()
{
	ObservedType = FItemFragment::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

void UItemRemoverProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FItemFragment> ItemFragments = Context.GetMutableFragmentView<FItemFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			BuildingSubsystem->ItemHashGrid.RemovePoint(Context.GetEntity(EntityIndex), ItemFragments[EntityIndex].OldLocation);
		}
	});
}

void UItemRemoverProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FItemFragment>(EMassFragmentAccess::None);
}

void UItemRemoverProcessor::Initialize(UObject& Owner)
{
	BuildingSubsystem = UWorld::GetSubsystem<URTSBuildingSubsystem>(Owner.GetWorld());
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
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FItemFragment& Item = ItemFragments[EntityIndex];
			const FVector& Location = Transforms[EntityIndex].GetTransform().GetLocation();
			const FVector2D Location2D(Location.X, Location.Y);
			
			BuildingSubsystem->ItemHashGrid.UpdatePoint(Context.GetEntity(EntityIndex), Item.OldLocation, Location2D);
			Item.OldLocation = Location2D;
		}
	});
}

void UItemProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FItemFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FItemAddedToGrid>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UItemProcessor::Initialize(UObject& Owner)
{
	BuildingSubsystem = UWorld::GetSubsystem<URTSBuildingSubsystem>(Owner.GetWorld());
}

void UItemInitializerProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		TArrayView<FItemFragment> ItemFragments = Context.GetMutableFragmentView<FItemFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FItemFragment& Item = ItemFragments[EntityIndex];
			const FTransform& Transform = Transforms[EntityIndex].GetTransform();
			const FVector& Location = Transform.GetLocation();
			const FVector2D Location2D(Location.X, Location.Y);
			
			BuildingSubsystem->ItemHashGrid.InsertPoint(Context.GetEntity(EntityIndex), Location2D);
			Item.OldLocation = Location2D;
			
			Context.Defer().AddTag<FItemAddedToGrid>(Context.GetEntity(EntityIndex));
		}
	});
}
