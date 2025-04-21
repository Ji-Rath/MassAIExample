// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "RandomMovementFragments.h"

#include "RandomMovementTrait.generated.h"

/**
 * 
 */
UCLASS()
class RANDOMMOVEMENT_API URandomMovementTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRandomMovementSettingsFragment RandomMovementSettings;
};
