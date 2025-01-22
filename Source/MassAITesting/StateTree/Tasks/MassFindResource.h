// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSignalSubsystem.h"
#include "MassSmartObjectHandler.h"
#include "MassSmartObjectRequest.h"
#include "MassStateTreeTypes.h"
#include "MassFindResource.generated.h"

struct FTransformFragment;
struct FResourceUserFragment;
/**
 * Task to assign a LookAt target for mass processing
 */
USTRUCT()
struct MASSAITESTING_API FMassFindResourceInstanceData
{
	GENERATED_BODY()
 
	/** Delay before the task ends. Default (0 or any negative) will run indefinitely so it requires a transition in the state tree to stop it. */
	UPROPERTY(EditAnywhere, Category = Output)
	bool bFoundSmartObject = false;
	
	UPROPERTY(EditAnywhere, Category = Output)
	FMassSmartObjectCandidateSlots FoundSlots;
	
	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTag WoodResourceTag;

	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTag RockResourceTag;
	
	FMassSmartObjectRequestID RequestID;
};

/**
 * 
 */
USTRUCT(meta = (DisplayName = "Mass Find Resource"))
struct MASSAITESTING_API FMassFindResource : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMassFindResourceInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassFindResourceInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	TStateTreeExternalDataHandle<FResourceUserFragment> MassResourceUserHandle;
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
	TStateTreeExternalDataHandle<USmartObjectSubsystem> SmartObjectSubsystemHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
	TStateTreeExternalDataHandle<FResourceUserFragment> ResourceUserHandle;
};
