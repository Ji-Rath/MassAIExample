// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomMovementProcessors.h"

#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "RandomMovementFragments.h"

URandomMovementProcessors::URandomMovementProcessors()
	: EntityQuery(*this)
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void URandomMovementProcessors::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FRandomMovementSettingsFragment>();
	EntityQuery.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URandomMovementProcessors::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const auto& Transforms = Context.GetFragmentView<FTransformFragment>();
		const auto& MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		const auto& RandomMovementSettings = Context.GetConstSharedFragment<FRandomMovementSettingsFragment>();
		auto SignalSubsystem = Context.GetMutableSubsystem<UMassSignalSubsystem>();

		for (int i=0; i < Context.GetNumEntities(); i++)
		{
			const auto& Transform = Transforms[i].GetTransform();
			auto& MoveTarget = MoveTargetFragments[i];
			
			// Update move target variables
			MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();
			MoveTarget.DistanceToGoal = FVector::Dist(Transform.GetLocation(), MoveTarget.Center);
			
			// Reached location, signal processor to find new location
			if (MoveTarget.GetCurrentAction() == EMassMovementAction::Move && MoveTarget.DistanceToGoal < 25.f)
			{
				SignalSubsystem->DelaySignalEntityDeferred(Context, RandomMovement::LocationReached, Context.GetEntity(i), RandomMovementSettings.NewLocationDelay);
				MoveTarget.CreateNewAction(EMassMovementAction::Stand, *Context.GetWorld());
			}
		}
	});
}

URandomMovementSignalProcessor::URandomMovementSignalProcessor() :
	EntityQuery(*this)
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void URandomMovementSignalProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FRandomMovementSettingsFragment>();
}

void URandomMovementSignalProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
                                                    FMassSignalNameLookup& EntitySignals)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
{
	const auto& Transforms = Context.GetFragmentView<FTransformFragment>();
	const auto& MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
	const auto& RandomMovementSettings = Context.GetConstSharedFragment<FRandomMovementSettingsFragment>();

	for (int i=0; i < Context.GetNumEntities(); i++)
	{
		const auto& Transform = Transforms[i].GetTransform();
		auto& MoveTarget = MoveTargetFragments[i];
		
		float RandomX = FMath::RandRange(-RandomMovementSettings.NewLocationRadius,RandomMovementSettings.NewLocationRadius);
		float RandomY = FMath::RandRange(-RandomMovementSettings.NewLocationRadius,RandomMovementSettings.NewLocationRadius);
		
		MoveTarget.Center = Transform.GetLocation() + FVector(RandomX, RandomY, 0.f);
		MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
		MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
	}
});
}

void URandomMovementSignalProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);

	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());

	SubscribeToSignal(*SignalSubsystem, RandomMovement::LocationReached);
}
