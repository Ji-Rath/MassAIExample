// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GPUSkinPublicDefs.h"
#include "Containers/StaticArray.h"

#include "AnimToTextureDataAsset.h"

namespace AnimToTexture_Private
{

template <uint16 NumInfluences>
struct TVertexSkinWeight
{
	TStaticArray<uint16, NumInfluences> MeshBoneIndices;
	TStaticArray<uint16, NumInfluences> BoneIndices;
	TStaticArray<uint8, NumInfluences>  BoneWeights;
};

/* Returns RefPose Vertex Positions */
int32 GetVertices(const USkeletalMesh& Mesh, const int32 LODIndex, 
	TArray<FVector3f>& OutPositions, TArray<FVector3f>& OutNormals);

/* Finds closest vertex to point */
int32 FindClosestVertex(const FVector3f& Point, const TArray<FVector3f>& Vertices);

/* VertexMapping between StaticMesh and SkeletalMesh Vertices.
*  Note: Size of Mapping is Number of MeshDescription Vertices.
*/
void GetStaticToSkeletalMapping(
	const UStaticMesh& StaticMesh, const int32 StaticLODIndex,
	const USkeletalMesh& SkeletalMesh, const int32 SkeletalLODIndex,
	TArray<FVector3f>& OutStaticVertices, TArray<FVector3f>& OutSkeletalVertices,
	TArray<int32>& OutStaticToSkeletalMapping);

/* Computes CPUSkinning at Pose */
void GetSkinnedVertices(USkeletalMeshComponent* MeshComponent, const int32 LODIndex,
	TArray<FVector3f>& OutPositions, TArray<FVector3f>& OutNormals);

/** Gets Skin Weights Data from SkeletalMeshComponent */
void GetSkinWeightsData(const USkeletalMesh& SkeletalMesh, const int32 LODIndex, TArray<TVertexSkinWeight<MAX_TOTAL_INFLUENCES>>& SkinWeights);

/** Gets Skin Weights Data from SkeletalMeshComponent, Reduce it to NumInfluences and Maps it to the StaticMesh */
void GetReducedSkinWeightsData(
	const USkeletalMesh& SkeletalMesh, const int32 SkeletalLODIndex, 
	const TArray<int32> StaticToSkelMapping,
	TArray<TVertexSkinWeight<4>>& SkinWeights);

/** Reduce Weights */
void ReduceSkinWeightsData(const TArray<TVertexSkinWeight<MAX_TOTAL_INFLUENCES>>& InSkinWeights, TArray<TVertexSkinWeight<4>>& OutSkinWeights);

/* Returns the 4-Higher Influences and Weights */
void GetMaxFourInfluences(const TVertexSkinWeight<MAX_TOTAL_INFLUENCES>& InSkinWeights, TVertexSkinWeight<4>& OutSkinWeights);

/* Returns Number of RawBones (no virtual bones)*/
int32 GetNumBones(const USkeletalMesh* SkeletalMesh);

/* Returns RefPose Bone Transforms.
   Only the RawBones are returned (no virtual bones)
   The returned transforms are in ComponentSpace */
void GetRefBoneTransforms(const USkeletalMesh* SkeletalMesh, TArray<FTransform>& OutTransforms);

/* Returns Bone exist in the RawBone list (no virtual bones) */
bool HasBone(const USkeletalMesh* SkeletalMesh, const FName& Bone);

/* Returns Bone Names. 
   Only the RawBones are returned (no virtual bones) */
void GetBoneNames(const USkeletalMesh* SkeletalMesh, TArray<FName>& OutNames);

/** Returns a Mapping between Slave and Master. The hierarchy is traversed upstream until a matching Bone is found. */
void GetSlaveBoneMap(const USkeletalMesh* MasterSkeletalMesh, const USkeletalMesh* SlaveSkeletalMesh, TArray<int32>& SlaveBoneMap);

/* Decomposes Transform in Translation and AxisAndAngle */
void DecomposeTransformation(const FTransform& Transform, FVector3f& Translation, FVector4& Rotation);
void DecomposeTransformations(const TArray<FTransform>& Transforms, TArray<FVector3f>& Translations, TArray<FVector4>& Rotations);

// ----------------------------------------------------------------------------

//template<>
//FORCEINLINE void GetMaxInfluences<1>(
//	const TVertexSkinWeight<MAX_TOTAL_INFLUENCES>& InSkinWeights, TVertexSkinWeight<4>& OutSkinWeights)
//{
//	// Reset Values
//	for (int32 Index = 0; Index < 4; ++Index)
//	{
//		OutSkinWeights.BoneWeights[Index] = 0;
//		OutSkinWeights.BoneIndices[Index] = 0;
//		OutSkinWeights.MeshBoneIndices[Index] = 0;
//	}
//
//	// Set first influence to 1
//	OutSkinWeights.BoneWeights[0] = 255; // TNumericLimits<uint8>::Max();
//
//	// Find 1st Influence (Highest)
//	uint8 MaxWeight = TNumericLimits<uint8>::Min();
//	for (int32 Index = 0; Index < MAX_TOTAL_INFLUENCES; ++Index)
//	{
//		if (InSkinWeights.BoneWeights[Index] > MaxWeight)
//		{
//			MaxWeight = InSkinWeights.BoneWeights[Index];
//			OutSkinWeights.BoneIndices[0] = InSkinWeights.BoneIndices[Index];
//			OutSkinWeights.MeshBoneIndices[0] = InSkinWeights.MeshBoneIndices[Index];
//		}
//	}
//}
//
//template<>
//FORCEINLINE void GetMaxInfluences<2>(
//	const TVertexSkinWeight<MAX_TOTAL_INFLUENCES>& InSkinWeights, TVertexSkinWeight<4>& OutSkinWeights)
//{
//	// Reset Values
//	for (int32 Index = 0; Index < 4; ++Index)
//	{
//		OutSkinWeights.BoneIndices[Index] = 0;
//		OutSkinWeights.BoneWeights[Index] = 0; 
//		OutSkinWeights.MeshBoneIndices[Index] = 0;
//	}
//	
//	// Find 1st Influence (Highest)
//	for (int32 Index = 0; Index < MAX_TOTAL_INFLUENCES; ++Index)
//	{
//		if (InSkinWeights.BoneWeights[Index] > OutSkinWeights.BoneWeights[0])
//		{
//			OutSkinWeights.BoneWeights[0] = InSkinWeights.BoneWeights[Index];
//			OutSkinWeights.BoneIndices[0] = InSkinWeights.BoneIndices[Index];
//			OutSkinWeights.MeshBoneIndices[0] = InSkinWeights.MeshBoneIndices[Index];
//		}
//	}
//
//	// Find 2nd Influence
//	for (int32 Index = 0; Index < MAX_TOTAL_INFLUENCES; ++Index)
//	{
//		if (InSkinWeights.BoneIndices[Index] != OutSkinWeights.BoneIndices[0])
//		{
//			if (InSkinWeights.BoneWeights[Index] > OutSkinWeights.BoneWeights[1])
//			{
//				OutSkinWeights.BoneWeights[1] = InSkinWeights.BoneWeights[Index];
//				OutSkinWeights.BoneIndices[1] = InSkinWeights.BoneIndices[Index];
//				OutSkinWeights.MeshBoneIndices[1] = InSkinWeights.MeshBoneIndices[Index];
//			}
//		}
//	}
//
//	// Normalize Weighs
//	const float NormFactor = 255.0f / (OutSkinWeights.BoneWeights[0] + OutSkinWeights.BoneWeights[1]);
//	OutSkinWeights.BoneWeights[0] = FMath::RoundToInt(OutSkinWeights.BoneWeights[0] * NormFactor);
//	OutSkinWeights.BoneWeights[1] = FMath::RoundToInt(OutSkinWeights.BoneWeights[1] * NormFactor);
//}

FORCEINLINE void GetMaxFourInfluences(
	const TVertexSkinWeight<MAX_TOTAL_INFLUENCES>& InSkinWeights, TVertexSkinWeight<4>& OutSkinWeights)
{
	// Reset Values
	for (int32 Index = 0; Index < 4; ++Index)
	{
		OutSkinWeights.BoneWeights[Index] = 0;
		OutSkinWeights.BoneIndices[Index] = 0;
		OutSkinWeights.MeshBoneIndices[Index] = 0;
	}

	// Find 1st Influence (Highest)
	for (int32 Index = 0; Index < MAX_TOTAL_INFLUENCES; ++Index)
	{
		if (InSkinWeights.BoneWeights[Index] > OutSkinWeights.BoneWeights[0])
		{
			OutSkinWeights.BoneWeights[0] = InSkinWeights.BoneWeights[Index];
			OutSkinWeights.BoneIndices[0] = InSkinWeights.BoneIndices[Index];
			OutSkinWeights.MeshBoneIndices[0] = InSkinWeights.MeshBoneIndices[Index];
		}
	}

	// Find 2nd Influence
	for (int32 Index = 0; Index < MAX_TOTAL_INFLUENCES; ++Index)
	{		
		if (InSkinWeights.BoneIndices[Index] != OutSkinWeights.BoneIndices[0])
		{
			if (InSkinWeights.BoneWeights[Index] > OutSkinWeights.BoneWeights[1])
			{
				OutSkinWeights.BoneWeights[1] = InSkinWeights.BoneWeights[Index];
				OutSkinWeights.BoneIndices[1] = InSkinWeights.BoneIndices[Index];
				OutSkinWeights.MeshBoneIndices[1] = InSkinWeights.MeshBoneIndices[Index];
			}
		}
	}

	// Find 3rd Influence
	for (int32 Index = 0; Index < MAX_TOTAL_INFLUENCES; ++Index)
	{
		if (InSkinWeights.BoneIndices[Index] != OutSkinWeights.BoneIndices[0] && InSkinWeights.BoneIndices[Index] != OutSkinWeights.BoneIndices[1])
		{
			if (InSkinWeights.BoneWeights[Index] > OutSkinWeights.BoneWeights[2])
			{
				OutSkinWeights.BoneWeights[2] = InSkinWeights.BoneWeights[Index];
				OutSkinWeights.BoneIndices[2] = InSkinWeights.BoneIndices[Index];
				OutSkinWeights.MeshBoneIndices[2] = InSkinWeights.MeshBoneIndices[Index];
			}
		}
	}

	// Find 4th Influence
	for (int32 Index = 0; Index < MAX_TOTAL_INFLUENCES; ++Index)
	{
		if (InSkinWeights.BoneIndices[Index] != OutSkinWeights.BoneIndices[0] && InSkinWeights.BoneIndices[Index] != OutSkinWeights.BoneIndices[1] && InSkinWeights.BoneIndices[Index] != OutSkinWeights.BoneIndices[2])
		{
			if (InSkinWeights.BoneWeights[Index] > OutSkinWeights.BoneWeights[3])
			{
				OutSkinWeights.BoneWeights[3] = InSkinWeights.BoneWeights[Index];
				OutSkinWeights.BoneIndices[3] = InSkinWeights.BoneIndices[Index];
				OutSkinWeights.MeshBoneIndices[3] = InSkinWeights.MeshBoneIndices[Index];
			}
		}
	}

	// Normalize Weighs
	const float NormFactor = 255.0f / (OutSkinWeights.BoneWeights[0] + OutSkinWeights.BoneWeights[1] + OutSkinWeights.BoneWeights[2] + OutSkinWeights.BoneWeights[3]);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		OutSkinWeights.BoneWeights[Index] = FMath::RoundToInt(OutSkinWeights.BoneWeights[Index] * NormFactor);
	}
}

} // end namespace AnimToTexture_Private
