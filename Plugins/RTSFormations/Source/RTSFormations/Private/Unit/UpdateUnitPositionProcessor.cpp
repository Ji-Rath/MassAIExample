// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/UpdateUnitPositionProcessor.h"

#include "MassExecutionContext.h"
#include "Unit/UnitFragments.h"
#include "MassSignalSubsystem.h"
#include "RTSFormationSubsystem.h"
#include "RTSSignals.h"
#include "ProfilingDebugging/ScopedTimers.h"

void UUnitProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddSharedRequirement<FUnitFragment>(EMassFragmentAccess::ReadWrite);
}

void UUnitProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityManager.ForEachSharedFragment<FUnitFragment>([&Context](FUnitFragment& UnitFragment)
	{
		if (UnitFragment.UnitSettings.Formation != EFormationType::Circle)
		{
			UnitFragment.InterpRotation = FRotator3f(FMath::RInterpConstantTo(FRotator(UnitFragment.InterpRotation), FRotator(UnitFragment.UnitRotation), Context.GetDeltaTimeSeconds(), 15.f));
		}
		
		UnitFragment.InterpDestination = FVector3f(FMath::VInterpConstantTo(FVector(UnitFragment.InterpDestination), FVector(UnitFragment.UnitDestination), Context.GetDeltaTimeSeconds(), 150.f));
	});
}
