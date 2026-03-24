// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/MassFTProcessor.h"

UMassFTProcessor::UMassFTProcessor()
{
	QueryBasedPruning = EMassQueryBasedPruning::Never;
	bAutoRegisterWithProcessingPhases = false;
}
