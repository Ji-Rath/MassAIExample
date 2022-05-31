// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSMovementTrait.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassNavigationFragments.h"
#include "MassSignalProcessorBase.h"
#include "RTSMovementSubsystem.h"
#include "SmartObjectSubsystem.h"

void URTSGatherResourceProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, ([this, &EntitySubsystem](FMassExecutionContext& Context)
	{
		const TConstArrayView<FRTSGatherResourceFragment> GatherResourceFragments = Context.GetFragmentView<FRTSGatherResourceFragment>();
		const TArrayView<FRTSAgentFragment> RTSAgentFragment = Context.GetMutableFragmentView<FRTSAgentFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FRTSGatherResourceFragment& ResourceFragment = GatherResourceFragments[EntityIndex];
			FRTSAgentFragment& RTSAgent = RTSAgentFragment[EntityIndex];

			// Add the given resource to the agent
			int& InventoryItem = RTSAgent.Inventory.FindOrAdd(ResourceFragment.Resource);
			InventoryItem += ResourceFragment.Amount;

			// Subtract from required resources
			InventoryItem = RTSAgent.RequiredResources.FindOrAdd(ResourceFragment.Resource);
			InventoryItem -= ResourceFragment.Amount;
			
			if (InventoryItem <= 0)
			{
				RTSAgent.RequiredResources.Remove(ResourceFragment.Resource);
			}

			// Remove fragment so we dont infinitely grant resources
			// TODO: Consider using tags rather than just removing the fragment now
			Context.Defer().RemoveFragment<FRTSGatherResourceFragment>(Context.GetEntity(EntityIndex));
		}
	}));
}

void URTSGatherResourceProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSGatherResourceFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRTSAgentFragment>(EMassFragmentAccess::ReadWrite);
}

void URTSGatherResourceProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

void URTSMovementTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	BuildContext.AddFragment<FRTSAgentFragment>();
	BuildContext.AddTag<FRTSAgent>();
}

URTSMovementProcessor::URTSMovementProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void URTSMovementProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, ([this](FMassExecutionContext& Context)
	{
		const TArrayView<FRTSAgentFragment> RTSMoveFragmentList = Context.GetMutableFragmentView<FRTSAgentFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargetList = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FRTSAgentFragment& RTSMoveFragment = RTSMoveFragmentList[EntityIndex];
			FMassMoveTargetFragment& MoveTarget = MoveTargetList[EntityIndex];
			FTransform& Transform = TransformList[EntityIndex].GetMutableTransform();

			MoveTarget.DistanceToGoal = (MoveTarget.Center - Transform.GetLocation()).Length();
			MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();

			if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
			{
				SignalSubsystem->SignalEntity(UE::Mass::Signals::FollowPointPathDone, Context.GetEntity(EntityIndex));
			}

			/*
			// Destination
			if (MoveTarget.DistanceToGoal <= 100.f || MoveTarget.Center == FVector::ZeroVector)
			{
				//Hacky initializer
				if (Transform.GetLocation().Equals(RTSMoveFragment.SpawnLocation, 100.f) || MoveTarget.Center == FVector::ZeroVector)
				{
					DrawDebugBox(GetWorld(), Transform.GetLocation(), FVector(50.f), FColor::Green, true, -1, 0, 10);
					RTSMoveFragment.WoodNeeded = 2;
					RTSMoveFragment.RockNeeded = 2;
					RTSMoveFragment.SpawnLocation = Transform.GetLocation();
				}

				// Destroy resource when finished
				if (RTSMoveFragment.ClaimedObject.IsValid())
				{
					if (const USmartObjectComponent* SmartObjectComp = SmartObjectSubsystem->GetSmartObjectComponent(RTSMoveFragment.ClaimedObject))
					{
						SmartObjectComp->GetOwner()->Destroy();
					}
					SmartObjectSubsystem->Release(RTSMoveFragment.ClaimedObject);
				}
				
				// Resource gathering
				if (RTSMoveFragment.WoodNeeded > 0 || RTSMoveFragment.RockNeeded > 0)
				{
					// Get appropriate resource
					FName SmartObjectTag;
					if (RTSMoveFragment.WoodNeeded > 0)
						SmartObjectTag = FName("Object.Tree");
					else if (RTSMoveFragment.RockNeeded > 0)
						SmartObjectTag = FName("Object.Rock");
				
					// Find smart object
					FSmartObjectRequest Request;
					Request.Filter.UserTags.AddTag(FGameplayTag::RequestGameplayTag(SmartObjectTag));
				
					Request.QueryBox = FBox(Transform.GetLocation()-5000.f, Transform.GetLocation()+5000.f);
					Request.Filter.BehaviorDefinitionClass = USmartObjectMassBehaviorDefinition::StaticClass();
					FSmartObjectRequestResult Result = SmartObjectSubsystem->FindSmartObject(Request);

					// If a smart object is found, navigate to it
					if (Result.IsValid())
					{
						MoveTarget.Center = SmartObjectSubsystem->GetSlotLocation(Result.SlotHandle).Get(FVector::ZeroVector);
						RTSMoveFragment.ClaimedObject = SmartObjectSubsystem->Claim(Result);

						// Gather resource (ahead of time, oops)
						if (SmartObjectTag.IsEqual(FName("Object.Tree")))
							RTSMoveFragment.WoodNeeded--;
						else
							RTSMoveFragment.RockNeeded--;
					}
					//MoveTarget.Center = RTSMovementSubsystem->TargetLocation;
				}
				else //Returning to spawn
				{
					MoveTarget.Center = RTSMoveFragment.SpawnLocation;
				}
			}

			// Other
			MoveTarget.Center.Z = 0;
			MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();
			MoveTarget.DistanceToGoal = (MoveTarget.Center - Transform.GetLocation()).Length();
			*/
		}
	}));
}

void URTSMovementProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSAgentFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FRTSAgent>(EMassFragmentPresence::All);
}

void URTSMovementProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	RTSMovementSubsystem = UWorld::GetSubsystem<URTSMovementSubsystem>(Owner.GetWorld());
	SmartObjectSubsystem = UWorld::GetSubsystem<USmartObjectSubsystem>(Owner.GetWorld());
	SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
}
