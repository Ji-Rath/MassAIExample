// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "VertexAnimLocomotionProcessor.generated.h"

USTRUCT()
struct FVertexAnimLocomotionFragment : public FMassFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	int IdleAnimIndex = 0;

	UPROPERTY(EditAnywhere)
	int RunAnimIndex = 0;

	// Speed threshhold for swapping between idle/run
	UPROPERTY(EditAnywhere)
	float SpeedThreshhold = 50.f;
};

UCLASS()
class VERTEXANIMCHARACTER_API UVertexAnimLocomotionProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UVertexAnimLocomotionProcessor();

	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};


