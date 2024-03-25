// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSmartObjectRequest.h"
#include "MassSmartObjectTypes.h"
#include "MassStateTreeTypes.h"
#include "SmartObjectSubsystem.h"
#include "MassStateTreeMoveToEntityHandle.generated.h"

class UMassEntitySubsystem;
class URTSBuildingSubsystem;
struct FMassMoveTargetFragment;
struct FTransformFragment;
struct FMassSmartObjectUserFragment;
class UMassSignalSubsystem;
struct FMassMovementParameters;
USTRUCT()
struct FMassStateTreeMoveToEntityHandleInstanceData
{
	GENERATED_BODY()
	
	/** Result of the candidates search request (Input) */
	UPROPERTY(VisibleAnywhere, Category = Input)
	FMassEntityHandle ItemHandle;
};

/**
 * 
 */
USTRUCT()
struct MASSAITESTING_API FMassStateTreeMoveToEntityHandle : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassStateTreeMoveToEntityHandleInstanceData::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	//TStateTreeInstanceDataPropertyHandle<FMassEntityHandle> EntityHandle;
	//TStateTreeInstanceDataPropertyHandle<EMassSmartObjectClaimResult> ClaimResultHandle;
	
	TStateTreeExternalDataHandle<USmartObjectSubsystem> SmartObjectSubsystemHandle;
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<FMassSmartObjectUserFragment> SOUserHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
	TStateTreeExternalDataHandle<FMassMovementParameters> MoveParametersHandle;
	TStateTreeExternalDataHandle<UMassEntitySubsystem> EntitySubsystemHandle;
	TStateTreeExternalDataHandle<URTSBuildingSubsystem> BuildingSubsystemHandle;
};
