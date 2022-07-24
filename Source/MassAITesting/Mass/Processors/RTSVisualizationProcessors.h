// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassLODCollectorProcessor.h"
#include "MassRepresentationProcessor.h"
#include "MassVisualizationLODProcessor.h"

#include "RTSVisualizationProcessors.generated.h"

/**
 * Example processor demonstrating how to use the representation module
 */
UCLASS()
class MASSAITESTING_API URTSVisualizationProcessor : public UMassVisualizationProcessor
{
	GENERATED_BODY()

public:

	URTSVisualizationProcessor()
	{
		bAutoRegisterWithProcessingPhases = true;
	}

};

UCLASS()
class MASSAITESTING_API URTSLODVisualizationProcessor : public UMassVisualizationLODProcessor
{
	GENERATED_BODY()

public:

	URTSLODVisualizationProcessor()
	{
		bAutoRegisterWithProcessingPhases = true;
	}

};

UCLASS()
class MASSAITESTING_API URTSLODCollectorProcessor : public UMassLODCollectorProcessor
{
	GENERATED_BODY()

public:

	URTSLODCollectorProcessor()
	{
		bAutoRegisterWithProcessingPhases = true;
	}
};

