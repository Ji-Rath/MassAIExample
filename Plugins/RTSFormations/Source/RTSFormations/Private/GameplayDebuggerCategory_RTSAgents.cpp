// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayDebuggerCategory_RTSAgents.h"

#include "DrawDebugHelpers.h"
#include "MassEntitySubsystem.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "MassEntityUtils.h"
#include "MassExecutionContext.h"
#include "GameFramework/PlayerController.h"
#include "Unit/UnitFragments.h"

FGameplayDebuggerCategory_RTSAgents::FGameplayDebuggerCategory_RTSAgents()
{
	bShowOnlyWithDebugActor = false;
	SetDataPackReplication<FRepData>(&DataPack);
}

void FGameplayDebuggerCategory_RTSAgents::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	if (OwnerPC)
	{
		auto& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*OwnerPC->GetWorld());

		FMassEntityQuery EntityQuery(EntityManager.AsShared());
		EntityQuery.AddSharedRequirement<FUnitFragment>(EMassFragmentAccess::ReadOnly);
		EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadOnly);

		int Units = 0;
		FUnitHandle UnitHandle;
		EntityManager.ForEachSharedFragment<FUnitFragment>([&Units, &UnitHandle](FUnitFragment& UnitFragment)
		{
			UnitHandle = UnitFragment.UnitHandle; //@todo we should be able to select which unit we want to visualize
			Units++;
		});
		
		DataPack.NumUnits = Units;

		TArray<FVector> Positions;
		FMassExecutionContext Context(EntityManager);
		EntityQuery.SetChunkFilter([&UnitHandle](const FMassExecutionContext& Context)
		{
			auto UnitFragment = Context.GetSharedFragment<FUnitFragment>();
			return UnitFragment.UnitHandle == UnitHandle;
		});
		
		EntityQuery.ForEachEntityChunk(Context, [&Positions](FMassExecutionContext& Context)
		{
			auto MoveTargetFragments = Context.GetFragmentView<FMassMoveTargetFragment>();
			for (auto Entity : Context.CreateEntityIterator())
			{
				Positions.Emplace(MoveTargetFragments[Entity].Center);
			}
		});

		DataPack.Positions = Positions;
	}
}

void FGameplayDebuggerCategory_RTSAgents::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	CanvasContext.Printf(TEXT("{yellow}Units: {white}%d"), DataPack.NumUnits);
	CanvasContext.Printf(TEXT("{yellow}Entities in Unit 0: {white}%d"), DataPack.Positions.Num());
	for(FVector& Pos : DataPack.Positions)
	{
		DrawDebugCanvasWireSphere(CanvasContext.Canvas.Get(), Pos, FColor::Yellow, 20.f, 5);
	}
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_RTSAgents::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_RTSAgents());
}

void FGameplayDebuggerCategory_RTSAgents::FRepData::Serialize(FArchive& Ar)
{
	Ar << Positions;
	Ar << NumUnits;
}

#endif // WITH_GAMEPLAY_DEBUGGER
