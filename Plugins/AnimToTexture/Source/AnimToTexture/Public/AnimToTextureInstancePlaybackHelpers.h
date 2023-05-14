// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/InstancedStaticMeshComponent.h"

#include "AnimToTextureInstancePlaybackHelpers.generated.h"

// Use floats to match custom floats of instanced static mesh
// We could pack a float w/ more parameters if desired
USTRUCT(BlueprintType)
struct FAnimToTextureAnimState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimToTexture")
	float StartFrame = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimToTexture")
	float NumFrames = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimToTexture")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimToTexture")
	float bLooping = 1.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimToTexture")
	float GlobalStartTime = 0.0f; 
};

USTRUCT(BlueprintType)
struct FAnimToTextureInstancePlaybackData
{
	GENERATED_USTRUCT_BODY()

	// Store prev state to allow blending of prev->current state in material
	// Uncomment this if we start blending states
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimToTexture")
	//FAnimToTextureAnimState PrevState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimToTexture")
	FAnimToTextureAnimState CurrentState;
};

USTRUCT(BlueprintType)
struct FAnimToTextureAnimationSyncData
{
	GENERATED_USTRUCT_BODY()

	// The time used for sync when transitioning from skeletal mesh to material animated static mesh.
	// World real time at the time of the transition
	float SyncTime;
};

USTRUCT(BlueprintType)
struct FAnimToTextureInstanceData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FAnimToTextureInstancePlaybackData> PlaybackData;

	UPROPERTY()
	TArray<FInstancedStaticMeshInstanceData> StaticMeshInstanceData;
};

class UAnimToTextureDataAsset;

UCLASS()
class ANIMTOTEXTURE_API UAnimToTextureInstancePlaybackLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Anim Texture Playback")
	static void SetupInstancedMeshComponent(UInstancedStaticMeshComponent* InstancedMeshComponent, UPARAM(ref) FAnimToTextureInstanceData& InstanceData, int32 NumInstances);

	UFUNCTION(BlueprintCallable, Category = "Anim Texture Playback")
	static void BatchUpdateInstancedMeshComponent(UInstancedStaticMeshComponent* InstancedMeshComponent, UPARAM(ref) FAnimToTextureInstanceData& InstanceData);

	UFUNCTION(BlueprintCallable, Category = "Anim Texture Playback")
	static void AllocateInstanceData(UPARAM(ref) FAnimToTextureInstanceData& InstanceData, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "Anim Texture Playback")
	static bool UpdateInstanceData(UPARAM(ref) FAnimToTextureInstanceData& InstanceData, int32 InstanceIndex, const FAnimToTextureInstancePlaybackData& PlaybackData, const FTransform& Transform);

	UFUNCTION(BlueprintCallable, Category = "Anim Texture Playback")
	static bool GetInstancePlaybackData(UPARAM(ref) const FAnimToTextureInstanceData& InstanceData, int32 InstanceIndex, FAnimToTextureInstancePlaybackData& InstancePlaybackData);

	UFUNCTION(BlueprintCallable, Category = "Anim Texture Playback")
	static bool GetInstanceTransform(UPARAM(ref) const FAnimToTextureInstanceData& InstanceData, int32 InstanceIndex, FTransform& InstanceTransform);

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Anim Texture Playback")
	static bool AnimStateFromDataAsset(const UAnimToTextureDataAsset* DataAsset, int32 StateIndex, FAnimToTextureAnimState& AnimState);
};
