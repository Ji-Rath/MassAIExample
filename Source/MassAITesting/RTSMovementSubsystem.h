// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RTSMovementSubsystem.generated.h"

/**
 * Not currently used. Was simply a test to move entities to a target location
 */
UCLASS()
class MASSAITESTING_API URTSMovementSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetLocation;
};
