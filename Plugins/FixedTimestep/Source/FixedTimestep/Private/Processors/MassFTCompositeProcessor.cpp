// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/MassFTCompositeProcessor.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Data/FTFragments.h"

UMassFTCompositeProcessor::UMassFTCompositeProcessor()
{
	QueryBasedPruning = EMassQueryBasedPruning::Never;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Movement);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::FixedTimestep;
}

