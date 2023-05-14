// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimToTextureUtils.h"
#include "AnimToTextureSkeletalMesh.h"

#include "Rendering/SkeletalMeshRenderData.h"

namespace AnimToTexture_Private
{
	bool CheckSkinWeightsToTexture(const TArray<TVertexSkinWeight<4>>& SkinWeights)
	{
		bool bHasInvalidIndices = false;
		int16 InvalidMeshBoneIndex = INDEX_NONE;
		for (int32 VertexIndex = 0; VertexIndex < SkinWeights.Num(); ++VertexIndex)
		{
			// MeshBoneIndex can not be higher than 255
			for (uint16 MeshBoneIndex : SkinWeights[VertexIndex].MeshBoneIndices)
			{
				if (MeshBoneIndex > 255)
				{
					InvalidMeshBoneIndex = MeshBoneIndex;
					bHasInvalidIndices = true;
					break;
				}
			}

			if (InvalidMeshBoneIndex != INDEX_NONE)
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid SkinWeights on Vertex: %i. MeshBoneIndex: %i > 255"), VertexIndex, InvalidMeshBoneIndex)
			}
		}

		if (bHasInvalidIndices)
		{
			return false;
		}

		return true;
	}

	bool WriteSkinWeightsToTexture(const TArray<TVertexSkinWeight<4>>& SkinWeights,
		const int32 RowsPerFrame, const int32 Height, const int32 Width, UTexture2D* Texture)
	{
		if (!Texture)
		{
			return false;
		}

		// Sanity check for MeshBoneIndices > 256
		if (!CheckSkinWeightsToTexture(SkinWeights))
		{
			return false;
		}

		const int32 NumVertices = SkinWeights.Num();

		// Allocate PixelData.
		TArray<FColor> Pixels;
		Pixels.Init(FColor::Black, Height * Width);

		for (int32 VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
		{
			const TVertexSkinWeight<4>& VertexSkinWeight = SkinWeights[VertexIndex];

			// Write Influence
			// NOTE: we are assuming the bone index is under < 256
			Pixels[VertexIndex].R = (uint8)VertexSkinWeight.MeshBoneIndices[0];
			Pixels[VertexIndex].G = (uint8)VertexSkinWeight.MeshBoneIndices[1];
			Pixels[VertexIndex].B = (uint8)VertexSkinWeight.MeshBoneIndices[2];
			Pixels[VertexIndex].A = (uint8)VertexSkinWeight.MeshBoneIndices[3];
			
			// Write Weight
			Pixels[Width * RowsPerFrame + VertexIndex].R = VertexSkinWeight.BoneWeights[0];
			Pixels[Width * RowsPerFrame + VertexIndex].G = VertexSkinWeight.BoneWeights[1];
			Pixels[Width * RowsPerFrame + VertexIndex].B = VertexSkinWeight.BoneWeights[2];
			Pixels[Width * RowsPerFrame + VertexIndex].A = VertexSkinWeight.BoneWeights[3];
		};

		// Write to Texture
		return WriteToTexture<FLowPrecision>(Texture, Height, Width, Pixels);
	}

}