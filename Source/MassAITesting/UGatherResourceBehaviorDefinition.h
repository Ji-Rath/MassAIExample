// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSmartObjectBehaviorDefinition.h"
#include "RTSMovementTrait.h"
#include "UGatherResourceBehaviorDefinition.generated.h"

struct FMassBehaviorEntityContext;
/**
 * Used to gather a resource from a smart object
 */
UCLASS()
class MASSAITESTING_API UUGatherResourceBehaviorDefinition : public USmartObjectMassBehaviorDefinition
{
	GENERATED_BODY()

public:
	virtual void Activate(FMassCommandBuffer& CommandBuffer, const FMassBehaviorEntityContext& EntityContext) const override;

	virtual void Deactivate(FMassCommandBuffer& CommandBuffer, const FMassBehaviorEntityContext& EntityContext) const override;
	
	UPROPERTY(EditDefaultsOnly, Category = SmartObject)
	int ResourceAmount = 1;
	
	UPROPERTY(EditDefaultsOnly, Category = SmartObject)
	TEnumAsByte<EResourceType> ResourceType = Tree;
};
