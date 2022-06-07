// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSmartObjectBehaviorDefinition.h"
#include "UObject/Object.h"
#include "ConstructLevelBehaviorDefinition.generated.h"

struct FMassBehaviorEntityContext;
struct FMassCommandBuffer;
/**
 * 
 */
UCLASS()
class MASSAITESTING_API UConstructLevelBehaviorDefinition : public USmartObjectMassBehaviorDefinition
{
	GENERATED_BODY()
	
public:
	virtual void Activate(FMassCommandBuffer& CommandBuffer, const FMassBehaviorEntityContext& EntityContext) const override;

	virtual void Deactivate(FMassCommandBuffer& CommandBuffer, const FMassBehaviorEntityContext& EntityContext) const override;
};
