// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSmartObjectBehaviorDefinition.h"
#include "SmartObjectResourceDefinition.generated.h"

/**
 * 
 */
UCLASS()
class WORLDRESOURCES_API USmartObjectResourceDefinition : public USmartObjectMassBehaviorDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer ResourceTags;

	UPROPERTY(EditAnywhere)
	bool bRemoveTags = false;
	
	virtual void Activate(FMassCommandBuffer& CommandBuffer, const FMassBehaviorEntityContext& EntityContext) const override;
};
