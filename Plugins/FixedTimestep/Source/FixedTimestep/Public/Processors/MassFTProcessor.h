// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/MassEntity/Public/MassProcessor.h"
#include "MassFTProcessor.generated.h"

/**
 * Base class for all Fixed timestep processors. Will be automatically registered to the fixed timestep subsystem.
 */
UCLASS(Abstract)
class FIXEDTIMESTEP_API UMassFTProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UMassFTProcessor();
};
