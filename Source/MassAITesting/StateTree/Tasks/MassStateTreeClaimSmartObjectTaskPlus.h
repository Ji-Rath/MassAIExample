// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSmartObjectRequest.h"
#include "MassSmartObjectTypes.h"
#include "MassStateTreeTypes.h"
#include "SmartObjectSubsystem.h"
#include "MassStateTreeClaimSmartObjectTaskPlus.generated.h"

class UMassSignalSubsystem;
struct FMassSmartObjectUserFragment;
USTRUCT()
struct MASSAITESTING_API FMassStateTreeClaimSmartObjectTaskInstanceData
{
	GENERATED_BODY()

	/** Result of the candidates search request (Input) */
	UPROPERTY(VisibleAnywhere, Category = Input)
	FSmartObjectHandle SOHandle;

	/** Result of the claim on potential candidates from the search results (Output) */
	//UPROPERTY(VisibleAnywhere, Category = Output)
	//EMassSmartObjectClaimResult ClaimResult = EMassSmartObjectClaimResult::Unset;
};

/**
 * 
 */
USTRUCT(DisplayName="Claim Smart Object Plus")
struct MASSAITESTING_API FMassStateTreeClaimSmartObjectTaskPlus : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMassStateTreeClaimSmartObjectTaskInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
    virtual const UStruct* GetInstanceDataType() const override { return FMassStateTreeClaimSmartObjectTaskInstanceData::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	TStateTreeExternalDataHandle<FMassSmartObjectUserFragment> SmartObjectUserHandle;
	TStateTreeExternalDataHandle<USmartObjectSubsystem> SmartObjectSubsystemHandle;

};
