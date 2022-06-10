// Copyright 2019-2021 Rexocrates. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VertexAnimProfile.generated.h"

class UTexture2D;
class UStaticMesh;

// Struct Holding helper data specific to an Animation Sequence needed for the baking process
USTRUCT(BlueprintType)
struct VERTEXANIMTOOLSET_API FVASequenceData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = BakeSequence)
		UAnimationAsset* SequenceRef = NULL;
	
	UPROPERTY(EditAnywhere, Category = BakeSequence)
		int32 NumFrames = 8;

	UPROPERTY(EditAnywhere, Category = BakeSequenceGenerated)
		int32 AnimStart_Generated = 0;

	UPROPERTY(EditAnywhere, Category = BakeSequenceGenerated)
		float Speed_Generated = 1.f;
};

// Data asset holding all the helper data needed for the baking process
UCLASS(BlueprintType)
class VERTEXANIMTOOLSET_API UVertexAnimProfile : public UDataAsset
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, Category = AnimProfile)
		bool AutoSize = true;
	UPROPERTY(EditAnywhere, Category = AnimProfile)
	int32 MaxWidth = 2048;
	
	UPROPERTY(EditAnywhere, Category = VertAnim)
		bool UVMergeDuplicateVerts = true;
	UPROPERTY(EditAnywhere, Category = VertAnim)
	FIntPoint OverrideSize_Vert = FIntPoint(0, 0);
	UPROPERTY(EditAnywhere, Category = VertAnim)
	TArray <FVASequenceData> Anims_Vert;

	UPROPERTY(EditAnywhere, Category = BoneAnim)
		bool FullBoneSkinning = false;
	UPROPERTY(EditAnywhere, Category = BoneAnim)
	FIntPoint OverrideSize_Bone = FIntPoint(0, 0);
	UPROPERTY(EditAnywhere, Category = BoneAnim)
	TArray <FVASequenceData> Anims_Bone;

	UPROPERTY(EditAnywhere, Category = AnimProfileGenerated)
		UStaticMesh* StaticMesh = NULL;

	UPROPERTY(EditAnywhere, Category = Generated_VertAnim)
	int32 UVChannel_VertAnim = -1;
	UPROPERTY(EditAnywhere, Category = Generated_VertAnim)
	UTexture2D* OffsetsTexture = NULL;
	UPROPERTY(EditAnywhere, Category = Generated_VertAnim)
	UTexture2D* NormalsTexture = NULL;


	UPROPERTY(EditAnywhere, Category = Generated_VertAnim)
	int32 RowsPerFrame_Vert= 0;
	UPROPERTY(EditAnywhere, Category = Generated_VertAnim)
	float MaxValueOffset_Vert = 0;

	UPROPERTY(EditAnywhere, Category = Generated_BoneAnim)
		int32 UVChannel_BoneAnim = -1;
	UPROPERTY(EditAnywhere, Category = Generated_BoneAnim)
		int32 UVChannel_BoneAnim_Full = -1;
	UPROPERTY(EditAnywhere, Category = Generated_BoneAnim)
		UTexture2D* BonePosTexture = NULL;
	UPROPERTY(EditAnywhere, Category = Generated_BoneAnim)
		UTexture2D* BoneRotTexture = NULL;

	UPROPERTY(EditAnywhere, Category = Generated_BoneAnim)
		float MaxValuePosition_Bone = 0;

	int32 CalcTotalNumOfFrames_Vert() const;
	int32 CalcTotalRequiredHeight_Vert() const;

	int32 CalcTotalNumOfFrames_Bone() const;
	int32 CalcTotalRequiredHeight_Bone() const;

	int32 CalcStartHeightOfAnim_Vert(const int32 AnimIndex) const;
	int32 CalcStartHeightOfAnim_Bone(const int32 AnimIndex) const;

};
