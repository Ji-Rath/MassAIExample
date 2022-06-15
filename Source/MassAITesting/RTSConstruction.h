#pragma once
#include "MassObserverProcessor.h"
#include "RTSConstruction.generated.h"

class USmartObjectSubsystem;
class URTSBuildingSubsystem;
/**
 * Observer Processor to construct a smart object building floor
 */
UCLASS()
class MASSAITESTING_API URTSConstructBuilding : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSConstructBuilding();
	
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;

	TObjectPtr<URTSBuildingSubsystem> RTSMovementSubsystem;
	TObjectPtr<USmartObjectSubsystem> SmartObjectSubsystem;

	float IncrementHeight = 100.f;

	FMassEntityQuery EntityQuery;
};

/**
 * @brief Tag used to signal URTSConstructBuilding to construct a floor using data from FRTSBuildingFragment
 */
USTRUCT()
struct MASSAITESTING_API FRTSConstructFloor : public FMassTag
{
	GENERATED_BODY()
};