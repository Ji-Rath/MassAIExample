// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimToTextureDataAsset.h"

int32 UAnimToTextureDataAsset::GetIndexFromAnimSequence(const UAnimSequence* Sequence)
{
	int32 OutIndex = 0;

	int32 NumSequences = AnimSequences.Num();
	
	// We can store a sequence to index map for a faster search
	for (int32 CurrentIndex = 0; CurrentIndex < NumSequences; ++CurrentIndex)
	{
		const FAnimSequenceInfo& SequenceInfo = AnimSequences[CurrentIndex];
		if (SequenceInfo.AnimSequence == Sequence)
		{
			OutIndex = CurrentIndex;
			break;
		}
	}

	return OutIndex;
}

namespace AnimToTextureParamNames
{
	static const FName BoundingBoxMin = TEXT("MinBBox");
	static const FName BoundingBoxScale = TEXT("SizeBBox");
	static const FName RowsPerFrame = TEXT("RowsPerFrame");
	static const FName BoneWeightRowsPerFrame = TEXT("BoneWeightsRowsPerFrame");
	static const FName NumFrames = TEXT("NumFrames (S)");
	//static const FName UVChannel = TEXT("UVChannel");
	//static const FName AnimateSwitch = TEXT("Animate (B)");
	static const FName VertexPositionTexture = TEXT("PositionTexture");
	static const FName VertexNormalTexture = TEXT("NormalTexture");
	static const FName BonePositionTexture = TEXT("BonePositionTexture");
	static const FName BoneRotationTexture = TEXT("BoneRotationTexture");
	static const FName BoneWeightsTexture = TEXT("BoneWeightsTexture");
}

FAnimToTextureMaterialParamNames::FAnimToTextureMaterialParamNames()
{
	BoundingBoxMin = AnimToTextureParamNames::BoundingBoxMin;
	BoundingBoxScale = AnimToTextureParamNames::BoundingBoxScale;
	RowsPerFrame = AnimToTextureParamNames::RowsPerFrame;
	BoneWeightRowsPerFrame = AnimToTextureParamNames::BoneWeightRowsPerFrame;
	NumFrames = AnimToTextureParamNames::NumFrames;
	//UVChannel = AnimToTextureParamNames::UVChannel;
	//AnimateSwitch = AnimToTextureParamNames::AnimateSwitch;
	VertexPositionTexture = AnimToTextureParamNames::VertexPositionTexture;
	VertexNormalTexture = AnimToTextureParamNames::VertexNormalTexture;
	BonePositionTexture = AnimToTextureParamNames::BonePositionTexture;
	BoneRotationTexture = AnimToTextureParamNames::BoneRotationTexture;
	BoneWeightsTexture = AnimToTextureParamNames::BoneWeightsTexture;
}
