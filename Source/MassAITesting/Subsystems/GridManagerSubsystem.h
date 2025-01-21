// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSubsystemBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "GridManagerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MASSAITESTING_API UGridManagerSubsystem : public UMassSubsystemBase
{
	GENERATED_BODY()

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

public:
	UFUNCTION(BlueprintCallable)
	FVector GetClosestNode(const FVector& Position) const;

private:
	UPROPERTY()
	UInstancedStaticMeshComponent* GridMesh;
};

template<>
struct TMassExternalSubsystemTraits<UGridManagerSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
