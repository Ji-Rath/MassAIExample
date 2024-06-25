// Fill out your copyright notice in the Description page of Project Settings.


#include "SmartObjects/BenchSOBehavior.h"

#include "MassSmartObjectAI.h"

DEFINE_LOG_CATEGORY(LogMassSmartObjectAI);

void UBenchSOBehavior::Activate(FMassCommandBuffer& CommandBuffer,
	const FMassBehaviorEntityContext& EntityContext) const
{
	Super::Activate(CommandBuffer, EntityContext);

	// This is where you would implement logic that you want run when an entity interacts with a smart object
	UE_LOG(LogMassSmartObjectAI, Verbose, TEXT("Ran Bench behavior on %llu"), EntityContext.EntityView.GetEntity().AsNumber());
}
