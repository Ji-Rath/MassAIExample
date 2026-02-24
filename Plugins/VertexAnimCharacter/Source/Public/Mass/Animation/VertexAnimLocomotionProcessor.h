// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "VertexAnimLocomotionProcessor.generated.h"

UCLASS()
class VERTEXANIMCHARACTER_API UVertexAnimLocomotionProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UVertexAnimLocomotionProcessor();

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};


