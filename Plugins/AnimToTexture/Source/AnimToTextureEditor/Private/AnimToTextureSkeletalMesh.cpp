// Copyright Epic Games, Inc. All Rights Reserved.
#include "AnimToTextureSkeletalMesh.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "MeshDescription.h"
#include "BoneIndices.h"

namespace AnimToTexture_Private
{

int32 GetVertices(const USkeletalMesh& Mesh, const int32 LODIndex, 
	TArray<FVector3f>& OutPositions, TArray<FVector3f>& OutNormals)
{
	OutPositions.Reset();
	OutNormals.Reset();
	
	const FSkeletalMeshRenderData* RenderData = Mesh.GetResourceForRendering();
	check(RenderData);

	if (!RenderData->LODRenderData.IsValidIndex(LODIndex))
	{
		return INDEX_NONE;
	}

	// Get LOD Data
	const FSkeletalMeshLODRenderData& LODRenderData = RenderData->LODRenderData[LODIndex];

	// Get Total Num of Vertices (for all sections)
	const int32 NumVertices = LODRenderData.GetNumVertices();
	OutPositions.SetNumUninitialized(NumVertices);
	OutNormals.SetNumUninitialized(NumVertices);
	
	for (int32 VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
	{
		OutPositions[VertexIndex] = LODRenderData.StaticVertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex);
		OutNormals[VertexIndex] = LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex);
	};

	return NumVertices;
}

int32 FindClosestVertex(const FVector3f& Point, const TArray<FVector3f>& Vertices)
{
	float MinDistance = TNumericLimits<float>::Max();
	int32 ClosestIndex = INDEX_NONE;

	// Find Closest SkeletalMesh Vertex.
	for (int32 VertexIndex = 0; VertexIndex < Vertices.Num(); ++VertexIndex)
	{
		const float Distance = FVector3f::Dist(Point, Vertices[VertexIndex]);
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			ClosestIndex = VertexIndex;
		}
	}

	return ClosestIndex;
}

void GetStaticToSkeletalMapping(
	const UStaticMesh& StaticMesh, const int32 StaticLODIndex,
	const USkeletalMesh& SkeletalMesh, const int32 SkeletalLODIndex,
	TArray<FVector3f>& OutStaticVertices, TArray<FVector3f>& OutSkeletalVertices,
	TArray<int32>& OutStaticToSkeletalMapping)
{
	// Reset
	OutStaticVertices.Reset();
	OutSkeletalVertices.Reset();
	OutStaticToSkeletalMapping.Reset();

	// Get Mesh Description
	const FMeshDescription* MeshDescription = StaticMesh.GetMeshDescription(StaticLODIndex);
	check(MeshDescription);

	// Get SkeletalMesh Vertices
	TArray<FVector3f> Normals;
	const int32 SkelNumVertices = AnimToTexture_Private::GetVertices(SkeletalMesh, SkeletalLODIndex, OutSkeletalVertices, Normals);

	// Get MeshDescription Vertices
	const FVertexArray& Vertices = MeshDescription->Vertices();
	const int32 NumVertices = Vertices.Num();
	OutStaticVertices.SetNum(NumVertices);
	OutStaticToSkeletalMapping.SetNum(NumVertices);

	for (const FVertexID VertexID : Vertices.GetElementIDs())
	{
		const int32 VertexIndex = VertexID.GetValue();

		// Get Mesh Description Vertex Position
		const FVector3f Vertex = MeshDescription->GetVertexPosition(VertexID);
		OutStaticVertices[VertexIndex] = Vertex;

		// Find Closest SkeletalMesh Vertex
		OutStaticToSkeletalMapping[VertexIndex] = FindClosestVertex(Vertex, OutSkeletalVertices);
	}
}

int32 GetNumBones(const USkeletalMesh* SkeletalMesh)
{
	check(SkeletalMesh);
	
	const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
	return RefSkeleton.GetRawBoneNum();
}

/**
* Copied from FAnimationRuntime::GetComponentSpaceTransform
*/
void GetRefBoneTransforms(const USkeletalMesh* SkeletalMesh, TArray<FTransform>& Transforms)
{
	check(SkeletalMesh);
	Transforms.Reset();

	// Get Reference Skeleton
	const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
	const TArray<FTransform>& BoneSpaceTransforms = RefSkeleton.GetRawRefBonePose(); // Get only raw bones (no virtual)
	
	const int32 NumTransforms = BoneSpaceTransforms.Num();
	Transforms.SetNumUninitialized(NumTransforms);

	for (int32 BoneIndex = 0; BoneIndex < NumTransforms; ++BoneIndex)
	{
		// initialize to identity since some of them don't have tracks
		int32 IterBoneIndex = BoneIndex;
		FTransform ComponentSpaceTransform = BoneSpaceTransforms[BoneIndex];

		do
		{
			//const int32 ParentIndex = RefSkeleton.GetParentIndex(IterBoneIndex);
			const int32 ParentIndex = RefSkeleton.GetRawParentIndex(IterBoneIndex); // Get only raw bones (no virtual)
			if (ParentIndex != INDEX_NONE)
			{
				ComponentSpaceTransform *= BoneSpaceTransforms[ParentIndex];
			}

			IterBoneIndex = ParentIndex;
		} while (RefSkeleton.IsValidIndex(IterBoneIndex));

		// 
		Transforms[BoneIndex] = ComponentSpaceTransform;
	}
}

bool HasBone(const USkeletalMesh* SkeletalMesh, const FName& Bone)
{
	check(SkeletalMesh);
	
	const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();

	for (const FMeshBoneInfo& BoneInfo: RefSkeleton.GetRawRefBoneInfo())
	{
		if (BoneInfo.Name == Bone)
		{
			return true;
		}
	}
	return false;
}


void GetBoneNames(const USkeletalMesh* SkeletalMesh, TArray<FName>& OutNames)
{
	check(SkeletalMesh);
	OutNames.Reset();
	
	const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
	
	const int32 NumRawBones = RefSkeleton.GetRawBoneNum();
	OutNames.SetNumUninitialized(NumRawBones);

	for (int32 BoneIndex = 0; BoneIndex < NumRawBones; ++BoneIndex)
	{
		OutNames[BoneIndex] = RefSkeleton.GetBoneName(BoneIndex);
	}
};

void GetSkinnedVertices(USkeletalMeshComponent* MeshComponent, const int32 LODIndex, 
	TArray<FVector3f>& OutPositions, TArray<FVector3f>& OutNormals)
{
	OutPositions.Reset();
	OutNormals.Reset();

	if (!MeshComponent)
	{
		return;
	}

	// Get SkeletalMesh
	USkeletalMesh* SkeletalMesh = MeshComponent->SkeletalMesh;
	if (!SkeletalMesh)
	{
		return;
	}

	// Get Render Data 
	FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetResourceForRendering();
	if (!RenderData->LODRenderData.IsValidIndex(LODIndex))
	{
		return;
	};

	// Get Matrices
	TArray<FMatrix44f> RefToLocals;
	MeshComponent->CacheRefToLocalMatrices(RefToLocals);

	// ---------------------------------------------------------------------------------------------

	// Get Ref-Pose Vertices
	TArray<FVector3f> Vertices;
	TArray<FVector3f> Normals;
	const int32 NumVertices = GetVertices(*SkeletalMesh, LODIndex, Vertices, Normals);

	// TODO: Add Morph Deltas to Vertices.

	// Get Weights
	TArray<TVertexSkinWeight<MAX_TOTAL_INFLUENCES>> SkinWeights;
	GetSkinWeightsData(*SkeletalMesh, LODIndex, SkinWeights);

	OutPositions.SetNumUninitialized(NumVertices);
	OutNormals.SetNumUninitialized(NumVertices);

	for (int32 VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
	{
		const FVector3f& Vertex = Vertices[VertexIndex];
		const FVector3f& Normal = Normals[VertexIndex];
		const TVertexSkinWeight<MAX_TOTAL_INFLUENCES>& Weights = SkinWeights[VertexIndex];

		FVector4f SkinnedVertex(0);
		FVector4f SkinnedNormal(0);

		for (int32 InfluenceIndex = 0; InfluenceIndex < MAX_TOTAL_INFLUENCES; InfluenceIndex++)
		{
			const uint8& BoneWeight = Weights.BoneWeights[InfluenceIndex];
			const uint16& MeshBoneIndex = Weights.MeshBoneIndices[InfluenceIndex];

			// Get Matrix
			const FMatrix44f& RefToLocal = RefToLocals[MeshBoneIndex];

			const float Weight = (float)BoneWeight / 255.f;
			SkinnedVertex += RefToLocal.TransformPosition(Vertex) * Weight;
			SkinnedNormal += RefToLocal.TransformVector(Normal) * Weight;
		}

		OutPositions[VertexIndex] = SkinnedVertex;
		OutNormals[VertexIndex] = SkinnedNormal;
	};
	
};

void ReduceSkinWeightsData(const TArray<TVertexSkinWeight<MAX_TOTAL_INFLUENCES>>& InSkinWeights, TArray<TVertexSkinWeight<4>>& OutSkinWeights)
{
	OutSkinWeights.SetNumUninitialized(InSkinWeights.Num());

	for (int32 VertexIndex = 0; VertexIndex < InSkinWeights.Num(); ++VertexIndex)
	{
		GetMaxFourInfluences(InSkinWeights[VertexIndex], OutSkinWeights[VertexIndex]);
	}
}

void GetSlaveBoneMap(const USkeletalMesh* MasterSkeletalMesh, const USkeletalMesh* SlaveSkeletalMesh, TArray<int32>& SlaveBoneMap)
{
	const TArray<FMeshBoneInfo>& MasterBoneInfo = MasterSkeletalMesh->GetRefSkeleton().GetRawRefBoneInfo();
	const TArray<FMeshBoneInfo>& SlaveBoneInfo = SlaveSkeletalMesh->GetRefSkeleton().GetRawRefBoneInfo();

	// Initialize Array
	const int32 MasterNumBones = MasterBoneInfo.Num();
	const int32 SlaveNumBones = SlaveBoneInfo.Num();
	SlaveBoneMap.Init(INDEX_NONE, SlaveNumBones);

	for (int32 SlaveBoneIndex = 0; SlaveBoneIndex < SlaveNumBones; ++SlaveBoneIndex)
	{
		// Search by name-matching
		for (int32 MasterBoneIndex = 0; MasterBoneIndex < MasterNumBones; ++MasterBoneIndex)
		{
			// found by name-matching
			if (SlaveBoneInfo[SlaveBoneIndex].Name == MasterBoneInfo[MasterBoneIndex].Name)
			{
				SlaveBoneMap[SlaveBoneIndex] = MasterBoneIndex;
				break;
			}
		}

		// Search in upstream hierarchy
		if (SlaveBoneMap[SlaveBoneIndex] == INDEX_NONE)
		{
			bool FoundMasterBone = false;
			int32 ParentIndex = SlaveBoneInfo[SlaveBoneIndex].ParentIndex;
			while (!FoundMasterBone && ParentIndex != INDEX_NONE)
			{
				for (int32 MasterBoneIndex = 0; MasterBoneIndex < MasterNumBones; ++MasterBoneIndex)
				{
					if (SlaveBoneInfo[ParentIndex].Name == MasterBoneInfo[MasterBoneIndex].Name)
					{
						SlaveBoneMap[SlaveBoneIndex] = MasterBoneIndex;
						FoundMasterBone = true;
						break;
					}
				}

				ParentIndex = SlaveBoneInfo[ParentIndex].ParentIndex;
			}
		}	

		// SlaveBone not found in Master
		if (SlaveBoneMap[SlaveBoneIndex] == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("Unable to find BoneMapping for: %s"), *SlaveBoneInfo[SlaveBoneIndex].Name.ToString())
		}
	}
}

void DecomposeTransformation(const FTransform& Transform, FVector3f& Translation, FVector4& Rotation)
{
	// Get Translation
	Translation = (FVector3f)Transform.GetTranslation();

	// Get Rotation 
	const FQuat Quat = Transform.GetRotation();

	FVector Axis;
	float Angle;
	Quat.ToAxisAndAngle(Axis, Angle);

	Rotation = FVector4(Axis, Angle);
}

void DecomposeTransformations(const TArray<FTransform>& Transforms, TArray<FVector3f>& Translations, TArray<FVector4>& Rotations)
{
	const int32 NumTransforms = Transforms.Num();
	Translations.SetNumUninitialized(NumTransforms);
	Rotations.SetNumUninitialized(NumTransforms);

	for (int32 Index = 0; Index < NumTransforms; ++Index)
	{
		DecomposeTransformation(Transforms[Index], Translations[Index], Rotations[Index]);
	}
};


void GetSkinWeightsData(const USkeletalMesh& SkeletalMesh, const int32 LODIndex, TArray<TVertexSkinWeight<MAX_TOTAL_INFLUENCES>>& SkinWeights)
{
	// Reset Weights
	SkinWeights.Reset();

	// Get Render Data
	const FSkeletalMeshRenderData* RenderData = SkeletalMesh.GetResourceForRendering();
	check(RenderData);

	if (!RenderData->LODRenderData.IsValidIndex(LODIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid LODIndex: %i"), LODIndex)
		return;
	}

	// Get LOD Data
	const FSkeletalMeshLODRenderData& LODRenderData = RenderData->LODRenderData[LODIndex];

	// Get Weights
	const FSkinWeightVertexBuffer* SkinWeightVertexBuffer = LODRenderData.GetSkinWeightVertexBuffer();
	check(SkinWeightVertexBuffer);

	// Get Weights from Buffer.
	// this is size of number of vertices.
	TArray<FSkinWeightInfo> SkinWeightsInfo;
	SkinWeightVertexBuffer->GetSkinWeights(SkinWeightsInfo);

	// Allocated SkinWeightData
	SkinWeights.SetNumUninitialized(SkinWeightsInfo.Num());

	// Loop thru vertices
	for (int32 VertexIndex = 0; VertexIndex < SkinWeightsInfo.Num(); ++VertexIndex)
	{
		// Find Section From Global Vertex Index
		// NOTE: BoneMap is stored by Section.
		int32 OutSectionIndex;
		int32 OutSectionVertexIndex;
		LODRenderData.GetSectionFromVertexIndex(VertexIndex, OutSectionIndex, OutSectionVertexIndex);

		// Get Section for Vertex.
		const FSkelMeshRenderSection& RenderSection = LODRenderData.RenderSections[OutSectionIndex];
		
		// Get Vertex Weights
		const FSkinWeightInfo& SkinWeightInfo = SkinWeightsInfo[VertexIndex];

		// Store Weights
		for (int32 InfluenceIndex = 0; InfluenceIndex < MAX_TOTAL_INFLUENCES; ++InfluenceIndex)
		{
			const uint8& BoneWeight = SkinWeightInfo.InfluenceWeights[InfluenceIndex];
			const uint16& BoneIndex = SkinWeightInfo.InfluenceBones[InfluenceIndex];
			const uint16& MeshBoneIndex = RenderSection.BoneMap[BoneIndex];

			SkinWeights[VertexIndex].BoneWeights[InfluenceIndex] = BoneWeight;
			SkinWeights[VertexIndex].BoneIndices[InfluenceIndex] = BoneIndex;
			SkinWeights[VertexIndex].MeshBoneIndices[InfluenceIndex] = MeshBoneIndex;
		}
	}
}

void GetReducedSkinWeightsData(
	const USkeletalMesh& SkeletalMesh, const int32 SkeletalLODIndex, 
	const TArray<int32> StaticToSkelMapping, 
	TArray<TVertexSkinWeight<4>>& OutSkinWeights)
{	
	// Get SkeletalMesh Weights
	TArray<TVertexSkinWeight<MAX_TOTAL_INFLUENCES>> SkinWeights;
	AnimToTexture_Private::GetSkinWeightsData(SkeletalMesh, SkeletalLODIndex, SkinWeights);
	
	// Reduce SkeletalMesh Weights to 4 highest influences.
	TArray<TVertexSkinWeight<4>> ReducedSkinWeights;
	AnimToTexture_Private::ReduceSkinWeightsData(SkinWeights, ReducedSkinWeights);

	// Allocate StaticMesh Weights
	OutSkinWeights.SetNumUninitialized(StaticToSkelMapping.Num());

	// Loop thru SkeletalMesh Weights
	//uint16 InvalidMeshBoneIndex;
	//bool bIsValidMeshBoneIndex = true;
	for (int32 VertexIndex = 0; VertexIndex < StaticToSkelMapping.Num(); ++VertexIndex)
	{
		const int32 SkelVertexIndex = StaticToSkelMapping[VertexIndex];
		OutSkinWeights[VertexIndex] = ReducedSkinWeights[SkelVertexIndex];

		// MeshBoneIndex can not be higher than 255
		/*for (uint16 MeshBoneIndex: OutSkinWeights[VertexIndex].MeshBoneIndices)
		{
			if (MeshBoneIndex > 255)
			{
				InvalidMeshBoneIndex = MeshBoneIndex;
				bIsValidMeshBoneIndex = false;
				break;
			}
		}

		if (!bIsValidMeshBoneIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid SkinWeights on Vertex: %i. MeshBoneIndex: %i > 255"), VertexIndex, InvalidMeshBoneIndex)
			break;
		}*/
	}
}

} // end namespace AnimToTexture_Private