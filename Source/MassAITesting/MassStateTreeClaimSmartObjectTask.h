// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSmartObjectRequest.h"
#include "MassSmartObjectTypes.h"
#include "MassStateTreeTypes.h"
#include "SmartObjectSubsystem.h"
#include "MassStateTreeClaimSmartObjectTask.generated.h"

class UMassSignalSubsystem;
struct FMassSmartObjectUserFragment;
USTRUCT()
struct FMassStateTreeClaimSmartObjectTaskInstanceData
{
	GENERATED_BODY()

	/** Result of the candidates search request (Input) */
	UPROPERTY(VisibleAnywhere, Category = Input)
	FMassSmartObjectRequestResult SearchRequestResult;

	UPROPERTY(VisibleAnywhere, Category = Input)
	FSmartObjectRequestFilter Filter;

	/** Result of the claim on potential candidates from the search results (Output) */
	UPROPERTY(VisibleAnywhere, Category = Output)
	EMassSmartObjectClaimResult ClaimResult = EMassSmartObjectClaimResult::Unset;
};

/**
 * 
 */
USTRUCT(DisplayName="Claim Smart Object Plus")
struct MASSAITESTING_API FMassStateTreeClaimSmartObjectTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	virtual bool Link(FStateTreeLinker& Linker) override;
    virtual const UStruct* GetInstanceDataType() const override { return FMassStateTreeClaimSmartObjectTaskInstanceData::StaticStruct(); }
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const override;

	TStateTreeInstanceDataPropertyHandle<FMassSmartObjectRequestResult> RequestResultHandle;
	TStateTreeInstanceDataPropertyHandle<EMassSmartObjectClaimResult> ClaimResultHandle;
	TStateTreeInstanceDataPropertyHandle<FSmartObjectRequestFilter> RequestFilterHandle;

	TStateTreeExternalDataHandle<FMassSmartObjectUserFragment> SmartObjectUserHandle;
	TStateTreeExternalDataHandle<USmartObjectSubsystem> SmartObjectSubsystemHandle;

};
