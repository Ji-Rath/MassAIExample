// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSmartObjectBehaviorDefinition.h"

#include "BenchSOBehavior.generated.h"

/**
 * 
 */
UCLASS()
class MASSSMARTOBJECTAI_API UBenchSOBehavior : public USmartObjectMassBehaviorDefinition
{
	GENERATED_BODY()
protected:
	virtual void Activate(FMassCommandBuffer& CommandBuffer, const FMassBehaviorEntityContext& EntityContext) const override;
};
