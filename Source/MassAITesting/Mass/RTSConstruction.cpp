
#include "RTSConstruction.h"

#include "MassSmartObjectFragments.h"
#include "RTSAgentTrait.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "SmartObjectComponent.h"

//----------------------------------------------------------------------//
// URTSConstructBuilding
//----------------------------------------------------------------------//
URTSConstructBuilding::URTSConstructBuilding()
{
	ObservedType = FRTSConstructFloor::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void URTSConstructBuilding::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FRTSAgentFragment> RTSAgents = Context.GetMutableFragmentView<FRTSAgentFragment>();
		TConstArrayView<FMassSmartObjectUserFragment> SOUsers = Context.GetFragmentView<FMassSmartObjectUserFragment>();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FRTSAgentFragment& RTSAgent = RTSAgents[EntityIndex];
			const FMassSmartObjectUserFragment& SOUser = SOUsers[EntityIndex];
			
			if (const USmartObjectComponent* SmartObjectComponent = SmartObjectSubsystem->GetSmartObjectComponent(SOUser.ClaimHandle))
			{
				const AActor* Actor = SmartObjectComponent->GetOwner();
				UInstancedStaticMeshComponent* InstancedStaticMeshComp = Actor->FindComponentByClass<UInstancedStaticMeshComponent>();
				FTransform Transform;
				Transform.SetLocation(FVector(0,0,IncrementHeight*InstancedStaticMeshComp->GetInstanceCount()));
				InstancedStaticMeshComp->AddInstance(Transform);

				RTSAgent.BuildingHandle = FSmartObjectHandle::Invalid;
				Context.Defer().RemoveTag<FRTSConstructFloor>(Context.GetEntity(EntityIndex));

				int* ResourceAmount = RTSAgent.Inventory.Find(EResourceType::Rock);
				if (ResourceAmount)
					*ResourceAmount -= 1;
				ResourceAmount = RTSAgent.Inventory.Find(EResourceType::Tree);
				if (ResourceAmount)
					*ResourceAmount -= 1;
				
			}
		}
	});
}

void URTSConstructBuilding::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSAgentFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassSmartObjectUserFragment>(EMassFragmentAccess::ReadOnly);
}

void URTSConstructBuilding::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	SmartObjectSubsystem = UWorld::GetSubsystem<USmartObjectSubsystem>(Owner.GetWorld());
}