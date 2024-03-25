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
	Filter.BehaviorDefinitionClass = USmartObjectMassBehaviorDefinition::StaticClass();
	
	// Retrieve fragments and subsystems
	USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
	FMassSmartObjectUserFragment& SOUser = Context.GetExternalData(SmartObjectUserHandle);

	// Setup MassSmartObject handler and claim
	FSmartObjectClaimHandle ClaimHandle = SmartObjectSubsystem.Claim(SOHandle, Filter);

	if (!ClaimHandle.IsValid())
		return EStateTreeRunStatus::Failed;
	SOUser.InteractionHandle = ClaimHandle;
	SOUser.InteractionStatus = EMassSmartObjectInteractionStatus::Unset;
	const FTransform Transform = SmartObjectSubsystem.GetSlotTransform(SOUser.InteractionHandle).Get(FTransform::Identity);
	//@todo fix this
	//SOUser.TargetLocation = Transform.GetLocation();
	//SOUser.TargetDirection = Transform.GetRotation().Vector();

	//ClaimResult = EMassSmartObjectClaimResult::Succeeded;

	return EStateTreeRunStatus::Succeeded;
}
