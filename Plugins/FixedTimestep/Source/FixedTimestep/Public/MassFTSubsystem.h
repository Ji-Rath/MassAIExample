// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSimulationSubsystem.h"
#include "MassSubsystemBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassFTSubsystem.generated.h"

class UMassFTCompositeProcessor;
class UMassCompositeProcessor;
/**
 * 
 */
UCLASS()
class FIXEDTIMESTEP_API UMassFTSubsystem : public UMassSubsystemBase
{
	GENERATED_BODY()
	
public:
	float GetInterpolationAlpha() const;
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	void TickProcessors(float DeltaTime);

	UPROPERTY()
	UMassFTCompositeProcessor* FixedTimestepProcessors;
	
	UPROPERTY()
	float AccumulatedDeltaTime = 0.f;
	
	UPROPERTY()
	UMassSimulationSubsystem* MassSim;
};

template<>
struct TMassExternalSubsystemTraits<UMassFTSubsystem>
{
	enum
	{
		GameThreadOnly = false,
		ThreadSafeWrite = false,
	};
};
