// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassNavigationFragments.h"
#include "SmartObjectRuntime.h"
#include "MassAIBehavior/Public/MassStateTreeTypes.h"
#include "MassMoveToSOTask.generated.h"

struct FTransformFragment;
struct FNavMeshPathFragment;
struct FStateTreeExecutionContext;

/**
 * Task to assign a LookAt target for mass processing
 */
USTRUCT()
struct MASSSMARTOBJECTAI_API FMassMoveToSOTaskInstanceData
{
	GENERATED_BODY()
 
	/** Delay before the task ends. Default (0 or any negative) will run indefinitely so it requires a transition in the state tree to stop it. */
	UPROPERTY(EditAnywhere, Category = Input)
	FSmartObjectClaimHandle ClaimHandle;
};

/**
 * 
 */
USTRUCT(meta = (DisplayName = "Move to SO Task"))
struct MASSSMARTOBJECTAI_API FMassMoveToSOTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMassMoveToSOTaskInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassMoveToSOTaskInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
protected:
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<USmartObjectSubsystem> SmartObjectSubsystemHandle;
};
