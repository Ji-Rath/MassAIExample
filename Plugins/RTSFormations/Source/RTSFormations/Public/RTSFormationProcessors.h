#pragma once
#include "MassEntityQuery.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"
#include "RTSFormationProcessors.generated.h"

// Observer that runs when a unit is spawned. Its main purpose is to add entities to a unit array
// in the subsystem and cache the index for future use in URTSFormationUpdate
UCLASS()
class RTSFORMATIONS_API URTSFormationInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSFormationInitializer();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

// Observer that runs when an entity is destroyed. Cleans up the unit array and tells the last unit to take their place
UCLASS()
class RTSFORMATIONS_API URTSFormationDestroyer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSFormationDestroyer();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};


// Simple movement processor to get agents from a to b
UCLASS()
class RTSFORMATIONS_API URTSAgentMovement : public UMassProcessor
{
	GENERATED_BODY()
	
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery = FMassEntityQuery(*this);

	FMassEntityQuery FormationQuery = FMassEntityQuery(*this);
};

// Main bulk of formation logic. Calculates position of entities and sends it to the FMassMoveTargetFragment.
UCLASS()
class RTSFORMATIONS_API URTSFormationUpdate : public UMassSignalProcessorBase
{
	GENERATED_BODY()
	
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;
};