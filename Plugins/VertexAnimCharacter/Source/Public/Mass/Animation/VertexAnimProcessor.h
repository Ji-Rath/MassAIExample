// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimToTextureDataAsset.h"
#include "MassProcessor.h"
#include "MassRepresentationTypes.h"
#include "VertexAnimProcessor.generated.h"

struct FMassActorFragment;
struct FMassVelocityFragment;
// Holds simple animation data (borrowed from CitySample)
USTRUCT()
struct VERTEXANIMCHARACTER_API FVertexAnimInfoFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UAnimToTextureDataAsset> AnimToTextureData;
	
	float GlobalStartTime = 0.0f;
	float PlayRate = 1.0f;
	int32 AnimationStateIndex = 0;
	bool bSwappedThisFrame = false;
	bool bCustomAnimation = false;
	int AnimPosition = 0;
};

/**
 * Vertex Animation processor for handling vertex animation changes. Simply updates meshes data based on FVertexAnimInfoFragment
 */
UCLASS()
class VERTEXANIMCHARACTER_API UVertexAnimProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UVertexAnimProcessor();

	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	UFUNCTION()
	void UpdateISMVertexAnimation(FMassInstancedStaticMeshInfo& ISMInfo, FVertexAnimInfoFragment& AnimationData,
	                              float LODSignificance, float PrevLODSignificance, int32 NumFloatsToPad);

	UFUNCTION()
	void UpdateAnimInstance(const FMassVelocityFragment& VelocityFragment, const FMassActorFragment& ActorFragment);

	FMassEntityQuery EntityQuery;

	FMassEntityQuery UpdateAnimInstanceQuery;
};
