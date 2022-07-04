// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeClaimSmartObjectTaskPlus.h"

#include "MassSmartObjectBehaviorDefinition.h"
#include "MassSmartObjectFragments.h"
#include "SmartObjectSubsystem.h"
#include "StateTreeExecutionContext.h"

bool FMassStateTreeClaimSmartObjectTaskPlus::Link(FStateTreeLinker& Linker)
{
	Linker.LinkInstanceDataProperty(SmartObjectHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeClaimSmartObjectTaskInstanceData, SOHandle));
	Linker.LinkInstanceDataProperty(ClaimResultHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeClaimSmartObjectTaskInstanceData, ClaimResult));

	Linker.LinkExternalData(SmartObjectUserHandle);
	Linker.LinkExternalData(SmartObjectSubsystemHandle);

	return true;
}

EStateTreeRunStatus FMassStateTreeClaimSmartObjectTaskPlus::EnterState(FStateTreeExecutionContext& Context,
	const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const
{
	const FSmartObjectHandle& SOHandle = Context.GetInstanceData(SmartObjectHandle);
	EMassSmartObjectClaimResult& ClaimResult = Context.GetInstanceData(ClaimResultHandle);
	
	FSmartObjectRequestFilter Filter;
	Filter.BehaviorDefinitionClass = USmartObjectMassBehaviorDefinition::StaticClass();
	
	// Retrieve fragments and subsystems
	USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
	FMassSmartObjectUserFragment& SOUser = Context.GetExternalData(SmartObjectUserHandle);

	// Setup MassSmartObject handler and claim
	FSmartObjectClaimHandle ClaimHandle = SmartObjectSubsystem.Claim(SOHandle, Filter);

	if (!ClaimHandle.IsValid())
		return EStateTreeRunStatus::Failed;
	SOUser.ClaimHandle = ClaimHandle;
	SOUser.InteractionStatus = EMassSmartObjectInteractionStatus::Unset;
	const FTransform Transform = SmartObjectSubsystem.GetSlotTransform(SOUser.ClaimHandle).Get(FTransform::Identity);
	SOUser.TargetLocation = Transform.GetLocation();
	SOUser.TargetDirection = Transform.GetRotation().Vector();

	ClaimResult = EMassSmartObjectClaimResult::Succeeded;

	return EStateTreeRunStatus::Succeeded;
}
