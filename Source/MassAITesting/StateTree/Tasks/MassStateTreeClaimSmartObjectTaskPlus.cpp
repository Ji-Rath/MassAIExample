// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeClaimSmartObjectTaskPlus.h"

#include "MassSmartObjectBehaviorDefinition.h"
#include "MassSmartObjectFragments.h"
#include "SmartObjectSubsystem.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMassStateTreeClaimSmartObjectTaskPlus::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(SmartObjectUserHandle);
	Linker.LinkExternalData(SmartObjectSubsystemHandle);

	return true;
}

EStateTreeRunStatus FMassStateTreeClaimSmartObjectTaskPlus::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	const FSmartObjectHandle& SOHandle = InstanceData.SOHandle;
	//EMassSmartObjectClaimResult& ClaimResult = InstanceData.ClaimResult;
	
	FSmartObjectRequestFilter Filter;
	Filter.BehaviorDefinitionClasses.Add(USmartObjectMassBehaviorDefinition::StaticClass());
	
	// Retrieve fragments and subsystems
	USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
	FMassSmartObjectUserFragment& SOUser = Context.GetExternalData(SmartObjectUserHandle);

	// Setup MassSmartObject handler and claim
	TArray<FSmartObjectSlotHandle> SOSlots;
	SmartObjectSubsystem.FindSlots(SOHandle, Filter, SOSlots);
	if (!SOSlots.IsEmpty() && SmartObjectSubsystem.CanBeClaimed(SOSlots[0]))
	{
		InstanceData.ClaimHandle = SmartObjectSubsystem.MarkSlotAsClaimed(SOSlots[0]);
		
	}
	

	if (!InstanceData.ClaimHandle.IsValid())
		return EStateTreeRunStatus::Failed;
	//SOUser.InteractionHandle = InstanceData.ClaimHandle;
	//SOUser.InteractionStatus = EMassSmartObjectInteractionStatus::Unset;
	const FTransform Transform = SmartObjectSubsystem.GetSlotTransform(SOUser.InteractionHandle).Get(FTransform::Identity);
	//@todo fix this
	//SOUser.TargetLocation = Transform.GetLocation();
	//SOUser.TargetDirection = Transform.GetRotation().Vector();

	//ClaimResult = EMassSmartObjectClaimResult::Succeeded;

	return EStateTreeRunStatus::Succeeded;
}
