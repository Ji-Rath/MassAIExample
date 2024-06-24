// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "MW_GotoLocation.generated.h"

class UMassSignalSubsystem;
struct FTransformFragment;
struct FMassMoveTargetFragment;

USTRUCT()
struct MASSWANDERAI_API FMW_GotoLocationInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Input)
	FVector Destination;

	UPROPERTY(EditAnywhere, Category=Output)
	FVector AgentLocation;

	FMW_GotoLocationInstanceData() = default;
};

/**
 * Sets the target location that the entity should go to. Returns successful when the entity has reached the location
 */
USTRUCT(meta = (DisplayName = "MW Goto Location"))
struct MASSWANDERAI_API FMW_GotoLocation : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMW_GotoLocationInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMW_GotoLocationInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
protected:
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};
