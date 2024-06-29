// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSmartObjectBehaviorDefinition.h"

#include "BuildingSOBehavior.generated.h"

/**
 * 
 */
UCLASS()
class BUILDINGMANAGER_API UBuildingSOBehavior : public USmartObjectMassBehaviorDefinition
{
	GENERATED_BODY()

	virtual void Activate(FMassCommandBuffer& CommandBuffer, const FMassBehaviorEntityContext& EntityContext) const override;
};
