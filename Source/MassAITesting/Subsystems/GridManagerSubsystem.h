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
	void GetNearbyNodes(const FVector& Position, TArray<int32>& OutNodes, float Range = 300.f) const;
	
	UPROPERTY()
	UInstancedStaticMeshComponent* GridMesh;

	TSet<int32> ClaimedNodes;

	UFUNCTION(BlueprintCallable)
	void AddClaimedNode(int32 Node);
	
	UFUNCTION(BlueprintCallable)
	void RemoveClaimedNode(int32 Node);
};

template<>
struct TMassExternalSubsystemTraits<UGridManagerSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
