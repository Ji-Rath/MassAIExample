// Copyright 2019-2021 Rexocrates. All Rights Reserved.

#include "VertexAnimProfile.h"

#include "UObject/ObjectMacros.h"
#include "Runtime/Engine/Classes/Engine/CanvasRenderTarget2D.h"

#include "Runtime/Engine/Classes/Kismet/KismetRenderingLibrary.h"

#include "Rendering/SkeletalMeshModel.h"


int32 UVertexAnimProfile::CalcTotalNumOfFrames_Vert() const
{
	int32 Out = 0;

	for (int32 i = 0; i < Anims_Vert.Num(); i++)
	{
		Out += Anims_Vert[i].NumFrames;
	}

	return Out;
}

int32 UVertexAnimProfile::CalcTotalRequiredHeight_Vert() const
{
	return RowsPerFrame_Vert * CalcTotalNumOfFrames_Vert();
}

int32 UVertexAnimProfile::CalcTotalNumOfFrames_Bone() const
{
	int32 Out = 0;

	for (int32 i = 0; i < Anims_Bone.Num(); i++)
	{
		Out += Anims_Bone[i].NumFrames;
	}

	return Out;
}

int32 UVertexAnimProfile::CalcTotalRequiredHeight_Bone() const
{
	return CalcTotalNumOfFrames_Bone();
}

int32 UVertexAnimProfile::CalcStartHeightOfAnim_Vert(const int32 AnimIndex) const
{
	int32 Out = 0;

	for (int32 i = 0; i < AnimIndex; i++)
	{
		 Out += Anims_Vert[i].NumFrames;
	}
	
	return RowsPerFrame_Vert * Out;
}

int32 UVertexAnimProfile::CalcStartHeightOfAnim_Bone(const int32 AnimIndex) const
{
	int32 Out = 0;

	for (int32 i = 0; i < AnimIndex; i++)
	{
		Out += Anims_Bone[i].NumFrames;
	}

	return Out + 1;
}
