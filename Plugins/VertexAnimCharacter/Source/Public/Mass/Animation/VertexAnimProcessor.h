// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassRepresentationTypes.h"
#include "VertexAnimTrait.h"
#include "VertexAnimProcessor.generated.h"

struct FMassActorFragment;
struct FMassVelocityFragment;

/**
 * Vertex Animation processor for handling vertex animation changes. Simply updates meshes data based on FVertexAnimInfoFragment
 */
UCLASS()
class VERTEXANIMCHARACTER_API UVertexAnimProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UVertexAnimProcessor();

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	static void UpdateISMVertexAnimation(FMassInstancedStaticMeshInfo& ISMInfo, FVertexAnimInfoFragment& AnimationData,
	                                     const FVertexAnimSharedFragment& VertexAnimData, float LODSignificance, float PrevLODSignificance, int32 NumFloatsToPad);
	
	static void UpdateAnimInstance(const FMassVelocityFragment& VelocityFragment, const FMassActorFragment& ActorFragment);

	FMassEntityQuery EntityQuery;
	FMassEntityQuery UpdateAnimInstanceQuery;
	FMassEntityQuery UpdateMontageQuery;
	FMassEntityQuery UpdateMontagePositionQuery;
};
