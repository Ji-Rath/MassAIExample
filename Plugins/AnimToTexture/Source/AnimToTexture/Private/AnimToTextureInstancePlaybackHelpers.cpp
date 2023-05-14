// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimToTextureInstancePlaybackHelpers.h"
#include "AnimToTextureDataAsset.h"

void UAnimToTextureInstancePlaybackLibrary::SetupInstancedMeshComponent(UInstancedStaticMeshComponent* InstancedMeshComponent, FAnimToTextureInstanceData& InstanceData, int32 NumInstances)
{
	if (InstancedMeshComponent)
	{
		InstancedMeshComponent->NumCustomDataFloats = InstanceData.PlaybackData.GetTypeSize() / sizeof(float);
		InstancedMeshComponent->PerInstanceSMData.Reset();
		InstancedMeshComponent->PerInstanceSMData.AddDefaulted(NumInstances);
		InstancedMeshComponent->PreAllocateInstancesMemory(NumInstances);
		InstancedMeshComponent->PerInstanceSMCustomData.SetNumZeroed(NumInstances * InstancedMeshComponent->NumCustomDataFloats);
		AllocateInstanceData(InstanceData, NumInstances);
	}
}

void UAnimToTextureInstancePlaybackLibrary::BatchUpdateInstancedMeshComponent(UInstancedStaticMeshComponent* InstancedMeshComponent, FAnimToTextureInstanceData& InstanceData)
{
	SIZE_T CustomDataSizeToCopy = FMath::Min(InstanceData.PlaybackData.Num() * InstanceData.PlaybackData.GetTypeSize(), InstancedMeshComponent->PerInstanceSMCustomData.Num() * InstancedMeshComponent->PerInstanceSMCustomData.GetTypeSize());
	FMemory::Memcpy(InstancedMeshComponent->PerInstanceSMCustomData.GetData(), InstanceData.PlaybackData.GetData(), CustomDataSizeToCopy);

	int32 TransformsToCopy = FMath::Min(InstancedMeshComponent->GetNumRenderInstances(), InstanceData.StaticMeshInstanceData.Num());
	InstancedMeshComponent->BatchUpdateInstancesData(0, TransformsToCopy, InstanceData.StaticMeshInstanceData.GetData(), true, false);
}

void UAnimToTextureInstancePlaybackLibrary::AllocateInstanceData(FAnimToTextureInstanceData& InstanceData, int32 Count)
{
	InstanceData.StaticMeshInstanceData.AddDefaulted(Count);
	InstanceData.PlaybackData.AddDefaulted(Count);
}

bool UAnimToTextureInstancePlaybackLibrary::UpdateInstanceData(FAnimToTextureInstanceData& InstanceData, int32 InstanceIndex, const FAnimToTextureInstancePlaybackData& PlaybackData, const FTransform& Transform)
{
	if (InstanceData.PlaybackData.IsValidIndex(InstanceIndex) && InstanceData.StaticMeshInstanceData.IsValidIndex(InstanceIndex))
	{
		InstanceData.PlaybackData[InstanceIndex] = PlaybackData;
		InstanceData.StaticMeshInstanceData[InstanceIndex].Transform = Transform.ToMatrixWithScale();
		return true;
	}

	return false;
}

bool UAnimToTextureInstancePlaybackLibrary::GetInstancePlaybackData(const FAnimToTextureInstanceData& InstanceData, int32 InstanceIndex, FAnimToTextureInstancePlaybackData& InstancePlaybackData)
{
	if (InstanceData.PlaybackData.IsValidIndex(InstanceIndex))
	{
		InstancePlaybackData = InstanceData.PlaybackData[InstanceIndex];
		return true;
	}
	
	return false;
}

bool UAnimToTextureInstancePlaybackLibrary::GetInstanceTransform(const FAnimToTextureInstanceData& InstanceData, int32 InstanceIndex, FTransform& InstanceTransform)
{
	if (InstanceData.StaticMeshInstanceData.IsValidIndex(InstanceIndex))
	{
		InstanceTransform = FTransform(InstanceData.StaticMeshInstanceData[InstanceIndex].Transform);
		return true;
	}

	return false;
}

bool UAnimToTextureInstancePlaybackLibrary::AnimStateFromDataAsset(const UAnimToTextureDataAsset* DataAsset, int32 StateIndex, FAnimToTextureAnimState& AnimState)
{
	if (DataAsset && DataAsset->Animations.IsValidIndex(StateIndex))
	{
		const FAnimInfo& AnimInfo = DataAsset->Animations[StateIndex];
		AnimState.StartFrame = AnimInfo.AnimStart;
		AnimState.NumFrames = AnimInfo.NumFrames;
		AnimState.bLooping = AnimInfo.bLooping;

		return true;
	}

	AnimState = FAnimToTextureAnimState();
	return false;
}

