// Fill out your copyright notice in the Description page of Project Settings.


#include "VATEditorUtils.h"

#include "IPersonaToolkit.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshRenderData.h"

#include "Animation/DebugSkelMeshComponent.h"


#include "Textures/SlateIcon.h"
#include "Styling/SlateTypes.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "RawMesh.h"
#include "StaticMeshResources.h"
#include "MeshBuild.h"

#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshRenderData.h"

#include "Engine/SkeletalMesh.h"
#include "SkeletalRenderPublic.h"
#include "Runtime/Engine/Private/SkeletalRenderCPUSkin.h"

#include "Animation/MorphTarget.h"

#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"

#include "Toolkits/ToolkitManager.h"

#include "Dialogs/DlgPickAssetPath.h"
#include "AssetRegistryModule.h"

#include "VertexAnimProfile.h"


#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "Engine.h"

#include "Misc/FeedbackContext.h"
#include "Misc/MessageDialog.h"

#include "IPersonaPreviewScene.h"
#include "AssetViewerSettings.h"
#include "RenderingThread.h"

#include "Components/PoseableMeshComponent.h"

#include "AnimationRuntime.h"

//--------------------------------------
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

#include "VertexAnimUtils.h"

#include "Animation/AnimSequence.h"

#include "Kismet/KismetRenderingLibrary.h"

#include "PackageTools.h"

#include "Animation/AnimSequence.h"

#include "Rendering/SkinWeightVertexBuffer.h"


#include "Misc/App.h"
#include "RenderingThread.h"
#include "GameFramework/PlayerController.h"
#include "ContentStreaming.h"
#include "DrawDebugHelpers.h"
#include "UnrealEngine.h"
#include "SkeletalRenderPublic.h"
//#include "SkeletalRenderCPUSkin.h"
//#include "SkeletalRenderGPUSkin.h"
//#include "SkeletalRenderStatic.h"
#include "Animation/AnimStats.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "EngineGlobals.h"
#include "PrimitiveSceneProxy.h"
#include "Engine/CollisionProfile.h"
#include "Rendering/SkinWeightVertexBuffer.h"
#include "SkeletalMeshTypes.h"
#include "Animation/MorphTarget.h"
#include "AnimationRuntime.h"

#include "Animation/AnimSingleNodeInstance.h"

#include "MeshDescription.h"

#define LOCTEXT_NAMESPACE "VATEditorUtils"


static void MapSkinVerts(
	UVertexAnimProfile* InProfile, const TArray <FFinalSkinVertex>& SkinVerts,
	TArray <int32>& UniqueVertsSourceID, TArray <FVector2D>& OutUVSet_Vert)
{
	TArray <FVector3f> UniqueVerts;
	TArray <int32> UniqueID;
	UniqueID.SetNumZeroed(SkinVerts.Num());

	for (int32 i = 0; i < SkinVerts.Num(); i++)
	{
		int32 ID = INDEX_NONE;

		if (InProfile->UVMergeDuplicateVerts)
		{
			if (UniqueVerts.Find(SkinVerts[i].Position, ID))
			{
				UniqueID[i] = ID;
			}
			else
			{
				UniqueID[i] = UniqueVerts.Num();
				UniqueVerts.Add(SkinVerts[i].Position);
				UniqueVertsSourceID.Add(i);
			}
		}
		else
		{
			UniqueID[i] = UniqueVerts.Num();
			UniqueVerts.Add(SkinVerts[i].Position);
			UniqueVertsSourceID.Add(i);
		}
	}



	if (InProfile->AutoSize)
	{
		int32 XSize = FMath::Min(InProfile->MaxWidth, (int32)FMath::RoundUpToPowerOfTwo(UniqueVerts.Num()));
		InProfile->RowsPerFrame_Vert = FMath::CeilToInt((float)(UniqueVerts.Num()) / (float)(XSize));
		InProfile->OverrideSize_Vert = FIntPoint(
			XSize,
			FMath::RoundUpToPowerOfTwo(InProfile->CalcTotalRequiredHeight_Vert()));
	}
	else
	{
		InProfile->RowsPerFrame_Vert = //FMath::CeilToInt((float)(UniqueVerts.Num()) / (float)(InProfile->OverrideSize.X));
			FMath::RoundUpToPowerOfTwo((float)(UniqueVerts.Num()) / (float)(InProfile->OverrideSize_Vert.X));
	}

	const float XStep = 1.f / InProfile->OverrideSize_Vert.X;
	const float YStep = 1.f / InProfile->OverrideSize_Vert.Y;
	const FVector2D HalfStep = FVector2D(XStep, YStep) / 2;
	TArray <FVector2D> UniqueMappedUVs;
	UniqueMappedUVs.SetNum(UniqueVerts.Num());

	for (int32 i = 0; i < UniqueVerts.Num(); i++)
	{
		// I SWITCHED THESE to have the UVs lined horizontally.
		const int32 GridX = i % InProfile->OverrideSize_Vert.X;
		const int32 GridY = i / InProfile->OverrideSize_Vert.X;
		const FVector2D GridUV = FVector2D(GridX * XStep, GridY * YStep);
		UniqueMappedUVs[i] = GridUV;
	}

	TArray <FVector2D> NewUVSet_Vert;
	NewUVSet_Vert.SetNum(SkinVerts.Num());
	for (int32 i = 0; i < SkinVerts.Num(); i++)
	{
		NewUVSet_Vert[i] = UniqueMappedUVs[UniqueID[i]];
	}
	OutUVSet_Vert = NewUVSet_Vert;
};

static void MapActiveBones(
	UVertexAnimProfile* InProfile, const int32 NumBones, TArray <FVector2D>& OutUVSet_Bone)
{
	if (InProfile->AutoSize)
	{
		int32 XSize = FMath::Clamp((int32)FMath::RoundUpToPowerOfTwo(NumBones), 8, InProfile->MaxWidth);
		
		InProfile->OverrideSize_Bone = FIntPoint(
			XSize,
			FMath::RoundUpToPowerOfTwo(InProfile->CalcTotalRequiredHeight_Bone() + 1));
	}

	const float XStep = 1.f / InProfile->OverrideSize_Bone.X;
	const float YStep = 1.f / InProfile->OverrideSize_Bone.Y;
	TArray <FVector2D> UniqueMappedUVs;
	UniqueMappedUVs.SetNum(NumBones);

	for (int32 i = 0; i < NumBones; i++)
	{
		if (true)
		{
			// I SWITCHED THESE to have the UVs lined horizontally.
			const int32 GridX = i % InProfile->OverrideSize_Bone.X;
			const int32 GridY = i / InProfile->OverrideSize_Bone.X;
			const FVector2D GridUV = FVector2D(GridX * XStep, GridY * YStep);
			UniqueMappedUVs[i] = GridUV;
		}
		else
		{
			UniqueMappedUVs[i] = FVector2D();
		}
	}

	OutUVSet_Bone = UniqueMappedUVs;
};



static void SkinnedMeshVATData(
	USkinnedMeshComponent* InSkinnedMeshComponent,
	UVertexAnimProfile* InProfile,
	TArray <int32>& UniqueSourceID,
	TArray <TArray <FVector2D>>& UVs_VertAnim,
	TArray <TArray <FVector2D>>& UVs_BoneAnim1, 
	TArray <TArray <FVector2D>>& UVs_BoneAnim2,
	TArray <TArray <FColor>>& Colors_BoneAnim)
{
	UniqueSourceID.Empty();
	UVs_VertAnim.Empty();
	UVs_BoneAnim1.Empty();
	UVs_BoneAnim2.Empty();
	Colors_BoneAnim.Empty();

	const int32 NumLODs = InSkinnedMeshComponent->GetNumLODs();

	const auto& RefSkeleton = InSkinnedMeshComponent->SkeletalMesh->GetRefSkeleton();
	const auto& GlobalRefSkeleton = InSkinnedMeshComponent->SkeletalMesh->GetSkeleton()->GetReferenceSkeleton();

	TArray <FVector2D> GridUVs_Vert;
	TArray <FVector2D> GridUVs_Bone;

	TArray<FFinalSkinVertex> AnimMeshFinalVertices;
	int32 AnimMeshLOD = 0;


	int32 UVVertStart = -1;
	int32 UVBoneStart = -2;
	
	FSkeletalMeshRenderData& SkeletalMeshRenderData = InSkinnedMeshComponent->MeshObject->GetSkeletalMeshRenderData();
	
	{
		FSkeletalMeshLODRenderData& LODData = SkeletalMeshRenderData.LODRenderData[0];
		MapActiveBones(InProfile, GlobalRefSkeleton.GetNum(), GridUVs_Bone);

		InSkinnedMeshComponent->GetCPUSkinnedVertices(AnimMeshFinalVertices, AnimMeshLOD);
		MapSkinVerts(InProfile, AnimMeshFinalVertices, UniqueSourceID, GridUVs_Vert);

		int32 UVChannelStart = LODData.StaticVertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();
		UVVertStart = InProfile->Anims_Vert.Num() ? UVChannelStart : -1;
		InProfile->UVChannel_VertAnim = UVVertStart;
		UVBoneStart =
			InProfile->Anims_Bone.Num() ? (InProfile->Anims_Vert.Num() ? UVChannelStart + 1 : UVChannelStart) : -2;
		InProfile->UVChannel_BoneAnim = UVBoneStart;
		InProfile->UVChannel_BoneAnim_Full = ((UVBoneStart >= 0) && InProfile->FullBoneSkinning) ? UVBoneStart + 1 : -1;
	}


	for (int32 OverallLODIndex = 0; OverallLODIndex < NumLODs; OverallLODIndex++)
	{
		int32 LODIndexRead = FMath::Min(OverallLODIndex, NumLODs - 1);

		FSkeletalMeshLODInfo& SrcLODInfo = *(InSkinnedMeshComponent->SkeletalMesh->GetLODInfo(LODIndexRead));

		// Get the CPU skinned verts for this LOD, WAIT, if it changes LOD on each loop, does that not mean it changes??
		TArray<FFinalSkinVertex> FinalVertices;
		InSkinnedMeshComponent->GetCPUSkinnedVertices(FinalVertices, LODIndexRead);


		TArray <FColor> thisLODSkinWeightColor;
		// Here is where we find the correct grid UVs for this LOD
		TArray <FVector2D> thisLODGridUVs_Vert, thisLODGridUVs_Bone1, thisLODGridUVs_Bone2;

		thisLODGridUVs_Bone1.SetNum(FinalVertices.Num());
		thisLODGridUVs_Bone2.SetNum(FinalVertices.Num());
		thisLODSkinWeightColor.SetNum(FinalVertices.Num());

		if (OverallLODIndex == AnimMeshLOD)
		{
			thisLODGridUVs_Vert = GridUVs_Vert;
		}
		else
		{
			// Here we search
			thisLODGridUVs_Vert.SetNum(FinalVertices.Num());

			for (int32 o = 0; o < FinalVertices.Num(); o++)
			{
				const FVector3f Pos = FinalVertices[o].Position;
				float Lowest = MAX_FLT;
				int32 WinnerID = INDEX_NONE;

				for (int32 u = 0; u < UniqueSourceID.Num(); u++)
				{
					const FVector3f TargetPos = AnimMeshFinalVertices[UniqueSourceID[u]].Position;

					const float Dist = FVector3f::Dist(Pos, TargetPos);
					if (Dist < Lowest)
					{
						Lowest = Dist;
						WinnerID = UniqueSourceID[u];
					}
				}

				check(WinnerID != INDEX_NONE);

				thisLODGridUVs_Vert[o] = GridUVs_Vert[WinnerID];
			}
		}


		FSkeletalMeshModel* Resource = InSkinnedMeshComponent->SkeletalMesh->GetImportedModel();
		FSkeletalMeshLODRenderData& LODData = SkeletalMeshRenderData.LODRenderData[LODIndexRead];

		{
			const FSkinWeightVertexBuffer& SkinWeightVertexBuffer = *LODData.GetSkinWeightVertexBuffer();

			auto SkinData = LODData.GetSkinWeightVertexBuffer();
			check(SkinData->GetNumVertices() == FinalVertices.Num());

			for (int32 s = 0; s < (int32)SkinData->GetNumVertices(); s++)
			{
				int32 SectionIndex;
				int32 VertIndex;
				LODData.GetSectionFromVertexIndex(s, SectionIndex, VertIndex);
				check(SectionIndex < LODData.RenderSections.Num());
				const FSkelMeshRenderSection& Section = LODData.RenderSections[SectionIndex];
				const auto& SoftVert = Resource->LODModels[LODIndexRead].Sections[SectionIndex].SoftVertices[VertIndex];

				uint32 InfluenceBones[4] = {
						SoftVert.InfluenceBones[0],
						SoftVert.InfluenceBones[1],
						SoftVert.InfluenceBones[2],
						SoftVert.InfluenceBones[3]
				};

				uint8 InfluenceWeights[4] = {
						SoftVert.InfluenceWeights[0],
						SoftVert.InfluenceWeights[1],
						SoftVert.InfluenceWeights[2],
						SoftVert.InfluenceWeights[3]
				};

				const float Sum =
					((float)InfluenceWeights[0] / 255.f) + ((float)InfluenceWeights[1] / 255.f)
					+ ((float)InfluenceWeights[2] / 255.f) + ((float)InfluenceWeights[3] / 255.f);
				const float Rest = 1.f - Sum;

				FLinearColor W = FLinearColor(
					((float)InfluenceWeights[0] / 255.f) + Rest,
					((float)InfluenceWeights[1] / 255.f),
					((float)InfluenceWeights[2] / 255.f),
					((float)InfluenceWeights[3] / 255.f));


				thisLODSkinWeightColor[s] = W.ToFColor(false);

				{
					check(Section.BoneMap.IsValidIndex(InfluenceBones[0]));
					check(Section.BoneMap.IsValidIndex(InfluenceBones[1]));
					check(Section.BoneMap.IsValidIndex(InfluenceBones[2]));
					check(Section.BoneMap.IsValidIndex(InfluenceBones[3]));

					const int32 
						Bone0 = GlobalRefSkeleton.FindBoneIndex(RefSkeleton.GetBoneName(Section.BoneMap[InfluenceBones[0]])),
						Bone1 = GlobalRefSkeleton.FindBoneIndex(RefSkeleton.GetBoneName(Section.BoneMap[InfluenceBones[1]])),
						Bone2 = GlobalRefSkeleton.FindBoneIndex(RefSkeleton.GetBoneName(Section.BoneMap[InfluenceBones[2]])),
						Bone3 = GlobalRefSkeleton.FindBoneIndex(RefSkeleton.GetBoneName(Section.BoneMap[InfluenceBones[3]]));

					
					checkf(GridUVs_Bone.IsValidIndex(Bone0), TEXT("NUMY %i || %i"),
						GridUVs_Bone.Num(), LODData.ActiveBoneIndices.Num());
					checkf(GridUVs_Bone.IsValidIndex(Bone1), TEXT("NUMY %i || %i"),
						GridUVs_Bone.Num(), LODData.ActiveBoneIndices.Num());
					checkf(GridUVs_Bone.IsValidIndex(Bone2), TEXT("NUMY %i || %i"),
						GridUVs_Bone.Num(), LODData.ActiveBoneIndices.Num());
					checkf(GridUVs_Bone.IsValidIndex(Bone3), TEXT("NUMY %i || %i"),
						GridUVs_Bone.Num(), LODData.ActiveBoneIndices.Num());

					
					thisLODGridUVs_Bone1[s] = FVector2D(
						GridUVs_Bone[Bone0].X,
						GridUVs_Bone[Bone1].X);
					thisLODGridUVs_Bone2[s] = FVector2D(
						GridUVs_Bone[Bone2].X,
						GridUVs_Bone[Bone3].X);
				}
			}
		}

		UVs_VertAnim.Add(thisLODGridUVs_Vert);
		UVs_BoneAnim1.Add(thisLODGridUVs_Bone1);
		UVs_BoneAnim2.Add(thisLODGridUVs_Bone2);
		Colors_BoneAnim.Add(thisLODSkinWeightColor);
	}
}


static void QuatSave(FQuat& Q)
{
	// Make sure we have a non null SquareSum. It shouldn't happen with a quaternion, but better be safe.
	if (Q.SizeSquared() <= SMALL_NUMBER)
	{
		Q = FQuat::Identity;
	}
	else
	{
		// All transmitted quaternions *MUST BE* unit quaternions, in which case we can deduce the value of W.
		if (!ensure(Q.IsNormalized()))
		{
			Q.Normalize();
		}
	}
}

void GatherAndBakeAllAnimVertData(
	UVertexAnimProfile* Profile,
	UDebugSkelMeshComponent* PreviewComponent,
	const TArray <int32>& UniqueSourceIDs,
	TArray <FVector4>& OutGridVertPos, 
	TArray <FVector4>& OutGridVertNormal,
	TArray <FVector4>& OutGridBonePos,
	TArray <FVector4>& OutGridBoneRot)
{
	bool bCachedCPUSkinning = false;
	constexpr bool bRecreateRenderStateImmediately = true;
	// 1º switch to CPU skinning
	{
		const int32 InLODIndex = 0;
		{
			if (USkinnedMeshComponent* MasterPoseComponentPtr = PreviewComponent->MasterPoseComponent.Get())
			{
				MasterPoseComponentPtr->SetForcedLOD(InLODIndex + 1);
				MasterPoseComponentPtr->UpdateLODStatus();
				MasterPoseComponentPtr->RefreshBoneTransforms(nullptr);
			}
			else
			{
				PreviewComponent->SetForcedLOD(InLODIndex + 1);
				PreviewComponent->UpdateLODStatus();
				PreviewComponent->RefreshBoneTransforms(nullptr);
			}

			// switch to CPU skinning
			bCachedCPUSkinning = PreviewComponent->GetCPUSkinningEnabled();

			PreviewComponent->SetCPUSkinningEnabled(true, bRecreateRenderStateImmediately);

			check(PreviewComponent->MeshObject);
			check(PreviewComponent->MeshObject->IsCPUSkinned());
		}
	}

	// 2º Make Sure it in ref pose
	PreviewComponent->EnablePreview(true, NULL);
	PreviewComponent->RefreshBoneTransforms(nullptr);
	PreviewComponent->ClearMotionVector();
	FlushRenderingCommands();


	TArray <FFinalSkinVertex> RefPoseFinalVerts = static_cast<FSkeletalMeshObjectCPUSkin*>(PreviewComponent->MeshObject)->GetCachedFinalVertices();

	TArray <FVector4> GridVertPos;
	TArray <FVector4> GridVertNormal;

	TArray <FVector4> GridBonePos;
	TArray <FVector4> GridBoneRot;

	float MaxValueOffset = 0.f;

	float MaxValuePosBone = 0.f;

	const int32 PerFrameArrayNum_Vert = Profile->OverrideSize_Vert.X * Profile->RowsPerFrame_Vert;
	const int32 PerFrameArrayNum_Bone = Profile->OverrideSize_Bone.X;

	

	TArray <FVector4> ZeroedPos;
	ZeroedPos.SetNumZeroed(PerFrameArrayNum_Vert);
	TArray <FVector4> ZeroedNorm;
	ZeroedNorm.SetNumZeroed(PerFrameArrayNum_Vert);

	// YOW, need different sizes for vert and bone textures.
	TArray <FVector4> ZeroedBonePos;
	ZeroedBonePos.SetNumZeroed(PerFrameArrayNum_Bone);
	TArray <FVector4> ZeroedBoneRot;
	ZeroedBoneRot.SetNumZeroed(PerFrameArrayNum_Bone);

	FSkeletalMeshRenderData& SkeletalMeshRenderData = PreviewComponent->MeshObject->GetSkeletalMeshRenderData();
	FSkeletalMeshLODRenderData& LODData = SkeletalMeshRenderData.LODRenderData[0];
	const auto& ActiveBoneIndices = LODData.ActiveBoneIndices;
	TArray <FMatrix44f> RefToLocal;

	// 3º Store Values
	// Vert Anim
	if (Profile->Anims_Vert.Num())
	{
		for (int32 i = 0; i < Profile->Anims_Vert.Num(); i++)
		{
			PreviewComponent->EnablePreview(true, Profile->Anims_Vert[i].SequenceRef);
			UAnimSingleNodeInstance* SingleNodeInstance = PreviewComponent->GetSingleNodeInstance();

			const float Length = SingleNodeInstance->GetLength();
			const float Step_Vert = Length / Profile->Anims_Vert[i].NumFrames;

			Profile->Anims_Vert[i].Speed_Generated = 1.f / Length;
			Profile->Anims_Vert[i].AnimStart_Generated = Profile->CalcStartHeightOfAnim_Vert(i);

			{

				for (int32 j = 0; j < Profile->Anims_Vert[i].NumFrames; j++)
				{
					const float AnimTime = Step_Vert * j;

					PreviewComponent->SetPosition(AnimTime, false);
					PreviewComponent->RefreshBoneTransforms(nullptr);
					PreviewComponent->RecreateClothingActors();
					// Cloth Ticking
					for (int32 P = 0; P < 8; P++) PreviewComponent->GetWorld()->Tick(ELevelTick::LEVELTICK_All, Step_Vert);

					PreviewComponent->ClearMotionVector();

					FlushRenderingCommands();

					TArray <FFinalSkinVertex> FinalVerts;
					FinalVerts = static_cast<FSkeletalMeshObjectCPUSkin*>(PreviewComponent->MeshObject)->GetCachedFinalVertices();

					for (int32 k = 0; k < UniqueSourceIDs.Num(); k++)
					{
						const int32 IndexInZeroed = k;
						const int32 VertID = UniqueSourceIDs[k];
						const FVector3f Delta = FinalVerts[VertID].Position - RefPoseFinalVerts[VertID].Position;
						MaxValueOffset = FMath::Max(Delta.GetAbsMax(), MaxValueOffset);
						ZeroedPos[IndexInZeroed] = FVector4(Delta);

						const FVector3f DeltaNormal = FinalVerts[VertID].TangentZ.ToFVector3f() - RefPoseFinalVerts[VertID].TangentZ.ToFVector3f();
						ZeroedNorm[IndexInZeroed] = FVector4(DeltaNormal);
						
					}

					GridVertPos.Append(ZeroedPos);
					GridVertNormal.Append(ZeroedNorm);
				}
			}
		}
	}


	// Bone Anim
	if (Profile->Anims_Bone.Num())
	{
		const auto& RefSkeleton = PreviewComponent->SkeletalMesh->GetRefSkeleton();
		const auto& GlobalRefSkeleton = PreviewComponent->SkeletalMesh->GetSkeleton()->GetReferenceSkeleton();
		// Ref Pose in Row 0
		{
			PreviewComponent->EnablePreview(true, NULL);
			PreviewComponent->RefreshBoneTransforms(nullptr);
			PreviewComponent->ClearMotionVector();
			FlushRenderingCommands();
			PreviewComponent->CacheRefToLocalMatrices(RefToLocal);

			for (int32 B = 0; B < RefSkeleton.GetNum(); B++)
			{
				FTransform RefTM = FAnimationRuntime::GetComponentSpaceTransformRefPose(RefSkeleton, B);
				FQuat RefQuat = RefTM.GetRotation();
				QuatSave(RefQuat);
				const int32 GlobalID = GlobalRefSkeleton.FindBoneIndex(RefSkeleton.GetBoneName(B));
				ZeroedBonePos[GlobalID] = RefTM.GetLocation();
				ZeroedBoneRot[GlobalID] = FVector4(RefQuat.X, RefQuat.Y, RefQuat.Z, RefQuat.W);
				//UE_LOG(LogUnrealMath, Warning, TEXT("%s"), *ZeroedBonePos[B].ToString());
			}
			GridBonePos.Append(ZeroedBonePos);
			GridBoneRot.Append(ZeroedBoneRot);
		}

		for (int32 i = 0; i < Profile->Anims_Bone.Num(); i++)
		{
			PreviewComponent->EnablePreview(true, Profile->Anims_Bone[i].SequenceRef);
			UAnimSingleNodeInstance* SingleNodeInstance = PreviewComponent->GetSingleNodeInstance();

			const float Length = SingleNodeInstance->GetLength();
			const float Step_Bone = Length / Profile->Anims_Bone[i].NumFrames;

			Profile->Anims_Bone[i].Speed_Generated = 1.f / Length;
			Profile->Anims_Bone[i].AnimStart_Generated = Profile->CalcStartHeightOfAnim_Bone(i);

			for (int32 j = 0; j < Profile->Anims_Bone[i].NumFrames; j++)
			{
				const float AnimTime = Step_Bone * j;

				PreviewComponent->SetPosition(AnimTime, false);
				PreviewComponent->RefreshBoneTransforms(nullptr);
				PreviewComponent->ClearMotionVector();
				FlushRenderingCommands();


				PreviewComponent->CacheRefToLocalMatrices(RefToLocal);

				{
					for (int32 k = 0; k < RefToLocal.Num(); k++)
					{
						const int32 GlobalID = GlobalRefSkeleton.FindBoneIndex(RefSkeleton.GetBoneName(k));

						FVector3f Pos = RefToLocal[k].GetOrigin();
						ZeroedBonePos[GlobalID] = FVector4(Pos);

						MaxValuePosBone = FMath::Max(MaxValuePosBone, Pos.GetAbsMax());

						FQuat Q = FQuat(RefToLocal[k].ToQuat());
						QuatSave(Q);
						ZeroedBoneRot[GlobalID] = FVector4(Q.X, Q.Y, Q.Z, Q.W);
					}
				}

				GridBonePos.Append(ZeroedBonePos);
				GridBoneRot.Append(ZeroedBoneRot);
			}
		}
	}

	// 4º Put Mesh back into ref pose
	{
		PreviewComponent->EnablePreview(true, NULL);
		PreviewComponent->RefreshBoneTransforms(nullptr);

		PreviewComponent->ClearMotionVector();
		
		// switch back to non CPU skinning
		{
			// switch skinning mode, LOD etc. back
			PreviewComponent->SetForcedLOD(0);
			PreviewComponent->SetCPUSkinningEnabled(bCachedCPUSkinning, bRecreateRenderStateImmediately);
		}

		FlushRenderingCommands();
	}

	Profile->MaxValueOffset_Vert = MaxValueOffset;
	Profile->MaxValuePosition_Bone = MaxValuePosBone;

	Profile->MarkPackageDirty();

	OutGridVertPos = GridVertPos;
	OutGridVertNormal = GridVertNormal;
	OutGridBonePos = GridBonePos;
	OutGridBoneRot = GridBoneRot;
}


static void EncodeData_Vec(const TArray <FVector4>& VectorData, const float MaxValue, const bool HDR, TArray <FFloat16Color>& Data)
{
	for (int32 i = 0; i < VectorData.Num(); i++)
	{
		FVector VectorValue = VectorData[i];
		const float MaxDim = VectorValue.GetAbsMax();

		if (MaxDim > 0.f)
		{
			float Mag;
			if (HDR)
			{
				VectorValue.X = VectorValue.X / MaxDim;
				VectorValue.Y = VectorValue.Y / MaxDim;
				VectorValue.Z = VectorValue.Z / MaxDim;
				Mag = -1.0 + ((MaxDim / MaxValue) * 2.0);

				Data[i] = FLinearColor(VectorValue.X, VectorValue.Y, VectorValue.Z, Mag);
			}
			else
			{
				VectorValue.X = FVertexAnimUtils::EncodeFloat(VectorValue.X, MaxDim);
				VectorValue.Y = FVertexAnimUtils::EncodeFloat(VectorValue.Y, MaxDim);
				VectorValue.Z = FVertexAnimUtils::EncodeFloat(VectorValue.Z, MaxDim);
				Mag = MaxDim / MaxValue;

				Data[i] = FLinearColor(VectorValue.X, VectorValue.Y, VectorValue.Z, Mag);
			}
		}
	}
}

static void EncodeData_Quat(const bool HD, const TArray <FVector4>& VectorData, TArray <FFloat16Color>& Data)
{
	for (int32 i = 0; i < VectorData.Num(); i++)
	{
		FVector4 VectorValue = VectorData[i];
		uint8 BigComp = 0;
		float Max = -100.0;
		FVector WinnerValue;

		bool Bit0 = false;
		bool Bit1 = false;

		if (FMath::Abs(VectorValue[0]) > Max)
		{
			BigComp = 0;
			Bit0 = 0, Bit1 = 0;
			Max = FMath::Abs(VectorValue[0]);
			WinnerValue = FVector(VectorValue[1], VectorValue[2], VectorValue[3]);
		}
		if (FMath::Abs(VectorValue[1]) > Max)
		{
			BigComp = 1;
			Bit0 = 0, Bit1 = 1;
			Max = FMath::Abs(VectorValue[1]);
			WinnerValue = FVector(VectorValue[0], VectorValue[2], VectorValue[3]);
		}
		if (FMath::Abs(VectorValue[2]) > Max)
		{
			BigComp = 2;
			Bit0 = 1, Bit1 = 0;
			Max = FMath::Abs(VectorValue[2]);
			WinnerValue = FVector(VectorValue[0], VectorValue[1], VectorValue[3]);
		}
		if (FMath::Abs(VectorValue[3]) > Max)
		{
			BigComp = 3;
			Bit0 = 1, Bit1 = 1;
			Max = FMath::Abs(VectorValue[3]);
			WinnerValue = FVector(VectorValue[0], VectorValue[1], VectorValue[2]);
		}

		if (VectorValue[BigComp] < 0)
		{
			WinnerValue *= -1.0;
		}

		const float MaxDim = WinnerValue.GetAbsMax();
		// for now no bit based encoding, just have quats be always HDR (double precission).
		if (false)
		{
			/*
			if (MaxDim > 0.f)
			{
				FVector4 Encoded = FVertexAnimUtils::BitEncodeVecId_HD(WinnerValue, 1.0, BigComp);
				if(HD)
					Data[i] = FLinearColor(Encoded);
				else 
					Data[i] = FLinearColor(FVertexAnimUtils::BitEncodeVecId(WinnerValue, 1.0, BigComp));

				UE_LOG(LogUnrealMath, Warning, TEXT("Vec Value %s || Id %i || Result Encode %s"), 
					*WinnerValue.ToString(), BigComp, *Encoded.ToString());
			}
			else
			{
				Data[i] = FLinearColor(0, 0, 0, 1);
			}*/
		}
		else
		{
			if (MaxDim > 0.f) 
			{
			
				float R = FMath::Max(0.001f, FVertexAnimUtils::EncodeFloat(WinnerValue.X, MaxDim)) * (Bit0 ? 1.0 : -1.0);
				//R = FVertexAnimUtils::EncodeFloat(R * (Bit0 ? 1.0 : -1.0), 1.0);

				float G = FMath::Max(0.001f, FVertexAnimUtils::EncodeFloat(WinnerValue.Y, MaxDim)) * (Bit1 ? 1.0 : -1.0);
				//G = FVertexAnimUtils::EncodeFloat(G * (Bit1 ? 1.0 : -1.0), 1.0);

				float B = WinnerValue.Z / MaxDim;// FVertexAnimUtils::EncodeFloat(WinnerValue.Z, MaxDim);

				float A = -1.0 + ((MaxDim / 1.0) * 2.0);// 

				Data[i] = FLinearColor(R, G, B, A);
			}
			else
			{
				Data[i] = FLinearColor(0, 0, 0, 1);
			}
		}
	}
}

static UTexture2D* SetTexture2(
	UWorld* World, const FString PackagePath, const FString Name, 
	UTexture2D* Texture, 
	const int32 InSizeX, const int32 InSizeY,
	const TArray <FFloat16Color>& Data, //const TArray <FVector>& VectorData,
	EObjectFlags InObjectFlags)
{
	UTexture2D* NewTexture;

	{
		
		if (Texture != NULL)
		{
			NewTexture = NewObject<UTexture2D>(Texture->GetOuter(), FName(*Texture->GetName()), InObjectFlags);
			
		}
		else
		{
			UPackage* Package = CreatePackage(NULL, *(PackagePath + Name));
			check(Package);
			Package->FullyLoad();

			NewTexture = NewObject<UTexture2D>(Package, *Name, InObjectFlags);

			FAssetRegistryModule::AssetCreated(NewTexture);
		}

		checkf(NewTexture, TEXT("%s"), *Name);

		NewTexture->Source.Init(InSizeX, InSizeY, /*NumSlices=*/ 1, /*NumMips=*/ 1, TSF_RGBA16F);
		uint32* TextureData = (uint32*)NewTexture->Source.LockMip(0);
		const int32 TextureDataSize = NewTexture->Source.CalcMipSize(0);
		
		FMemory::Memcpy(TextureData, Data.GetData(), TextureDataSize); // this did not blow up 
		
		NewTexture->Source.UnlockMip(0);

		NewTexture->PostEditChange();

		NewTexture->MarkPackageDirty();
	}

	return NewTexture;
}

float FVATEditorUtils::PackBits(const uint32& bit)
{
	/*
	int32 f16 = bit;
	f16 += 1024;
	int32 sign = (f16 & (0x8000)) << 16;
	int32 expVar = ((((f16 >> 10) & 0x1f) - 15 + 127) << 23);
	int32 mant = (f16 & 0x3ff) << 13;
	f16 = (sign | expVar) | mant;
	return f16;
	*/

	uint32 f16 = static_cast<uint32>(bit);
	f16 += 1024;
	uint32 sign = (f16 & (0x8000)) << 16;
	uint32 expVar = //(f16 & 0x7fff) == 0 ? 0 : 
		(/*shift*/((/*bit. and*/ (/*bit.shift*/ f16 >> 10) & 0x1f) - 15 + 127) << 23);

	uint32 mant = (f16 & 0x3ff) << 13;
	f16 = /*bit. or*/ (/*bit. or*/ sign | expVar) | mant;
	//return f16;
	//UE_LOG(LogUnrealMath, Warning, TEXT("HH %i || %i"), bit, f16); // printing 0
	//return f16;// asfloat_O(f16);
	return static_cast<float>(f16);
}

int FVATEditorUtils::UnPackBits(const float N)
{
	uint32 uRes32 = static_cast<uint32>(N);// asuint(N);
	const uint32 sign2 = ((uRes32 >> 16) & 0x8000);
	const uint32 exp2 = ((((const int)((uRes32 >> 23) & 0xff)) - 127 + 15) << 10);
	const uint32 mant2 = ((uRes32 >> 13) & 0x3ff);
	const uint32 bits = (sign2 | exp2 | mant2);
	const uint32 result = bits - 1024;
	return float(result);
}

void FVATEditorUtils::DoBakeProcess(UDebugSkelMeshComponent* PreviewComponent)
{
	PreviewComponent->GlobalAnimRateScale = 0.f;
	
	UVertexAnimProfile* Profile = NULL;

	FString MeshName;
	FString PackageName;
	bool bOnlyCreateStaticMesh = false;

	{
		FString NewNameSuggestion = FString(TEXT("VertexAnimStaticMesh"));
		FString PackageNameSuggestion = FString(TEXT("/Game/Meshes/")) + NewNameSuggestion;
		FString Name;
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		AssetToolsModule.Get().CreateUniqueAssetName(PackageNameSuggestion, TEXT(""), PackageNameSuggestion, Name);

		//TSharedPtr<SDlgPickAssetPath> PickAssetPathWidget =
		TSharedPtr<SPickAssetDialog> PickAssetPathWidget =
			SNew(SPickAssetDialog)
			.Title(LOCTEXT("BakeAnimDialog", "Bake Anim Dialog"))
			.DefaultAssetPath(FText::FromString(PackageNameSuggestion));

		if (PickAssetPathWidget->ShowModal() == EAppReturnType::Ok)
		{
			// Get the full name of where we want to create the mesh asset.
			Profile = PickAssetPathWidget->GetSelectedProfile();
			bOnlyCreateStaticMesh = PickAssetPathWidget->GetOnlyCreateStaticMesh();

			PackageName = PickAssetPathWidget->GetFullAssetPath().ToString();
			MeshName = FPackageName::GetLongPackageAssetName(PackageName);

			// Check if the user inputed a valid asset name, if they did not, give it the generated default name
			if (MeshName.IsEmpty())
			{
				// Use the defaults that were already generated.
				PackageName = PackageNameSuggestion;
				MeshName = *Name;
			}
		}
	}


	bool DoAnimBake = (Profile != NULL) && !bOnlyCreateStaticMesh;
	bool DoStaticMesh = (Profile != NULL);

	if ((!DoAnimBake) && (!DoStaticMesh)) return;

	TArray <int32> UniqueSourceIDs;
	TArray <TArray <FVector2D>> UVs_VertAnim;
	TArray <TArray <FVector2D>> UVs_BoneAnim1;
	TArray <TArray <FVector2D>> UVs_BoneAnim2;
	TArray <TArray <FColor>> Colors_BoneAnim;

	{
		{
			SkinnedMeshVATData(
				PreviewComponent,
				Profile,
				UniqueSourceIDs,
				UVs_VertAnim,
				UVs_BoneAnim1,
				UVs_BoneAnim2,
				Colors_BoneAnim);
		}

		if ((Profile->CalcTotalRequiredHeight_Vert() > Profile->OverrideSize_Vert.Y) ||
			(Profile->CalcTotalRequiredHeight_Bone() > Profile->OverrideSize_Bone.Y))
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SelectedProfileRequiresMoreHeight", "Selected Profile Requires More Texture Height"));
			return;
		}

		if ((Profile->OverrideSize_Vert.GetMax() > 4096) ||
			(Profile->OverrideSize_Bone.GetMax() > 4096))
		{
			FMessageDialog::Open(EAppMsgType::Ok,
				LOCTEXT("TooMuch", "Warning: required texture size exceeds UE texture resolution limit, Mesh has too many vertices and/or Profile has too many animation frames"));
			return;
		}
	}


	if (DoStaticMesh)
	{
		if (Profile->StaticMesh && (!bOnlyCreateStaticMesh))
		{
			PackageName = Profile->StaticMesh->GetOutermost()->GetName();
		}
		else
		{
			FString AssetName = Profile->GetOutermost()->GetName();
			//FString Name = Profile->GetName();
			FString Name = PreviewComponent->SkeletalMesh->GetName();
			//FString AssetName = PreviewComponent->SkeletalMesh->GetOutermost()->GetName();
			const FString SanitizedBasePackageName = UPackageTools::SanitizePackageName(AssetName);
			const FString PackagePath = FPackageName::GetLongPackagePath(SanitizedBasePackageName) + TEXT("/") + Name + TEXT("_VAT");
			PackageName = PackagePath;
		}

		UStaticMesh* StaticMesh = FVertexAnimUtils::ConvertMeshesToStaticMesh( { PreviewComponent }, FTransform::Identity, PackageName);

		if (Profile->UVChannel_VertAnim != -1) FVertexAnimUtils::VATUVsToStaticMeshLODs(StaticMesh, Profile->UVChannel_VertAnim, UVs_VertAnim);
		if (Profile->UVChannel_BoneAnim != -1) FVertexAnimUtils::VATUVsToStaticMeshLODs(StaticMesh, Profile->UVChannel_BoneAnim, UVs_BoneAnim1);
		if (Profile->UVChannel_BoneAnim_Full != -1)
		{
			FVertexAnimUtils::VATUVsToStaticMeshLODs(StaticMesh, Profile->UVChannel_BoneAnim_Full, UVs_BoneAnim2);
			FVertexAnimUtils::VATColorsToStaticMeshLODs(StaticMesh, Colors_BoneAnim);
		}

		Profile->StaticMesh = StaticMesh;
		Profile->MarkPackageDirty();
	}

	

	if (DoAnimBake)
	{
		int32 TextureWidth_Vert = Profile->OverrideSize_Vert.X;
		int32 TextureHeight_Vert = Profile->OverrideSize_Vert.Y;
		int32 TextureWidth_Bone = Profile->OverrideSize_Bone.X;
		int32 TextureHeight_Bone = Profile->OverrideSize_Bone.Y;

		
		TArray <FVector4> VertPos, VertNormal, BonePos, BoneRot;
		GatherAndBakeAllAnimVertData(Profile, PreviewComponent, UniqueSourceIDs, VertPos, VertNormal, BonePos, BoneRot);

		FString AssetName = Profile->GetOutermost()->GetName();
		const FString SanitizedBasePackageName = UPackageTools::SanitizePackageName(AssetName);
		const FString PackagePath = FPackageName::GetLongPackagePath(SanitizedBasePackageName) + TEXT("/");

		// Vert Textures
		if(Profile->Anims_Vert.Num())
		{
			TArray <FFloat16Color> Data;
			Data.SetNumZeroed(TextureWidth_Vert * TextureHeight_Vert);


			{
				EncodeData_Vec(VertNormal, 2.f, false, Data); // decided on fixed 2.0 for simplicity

				Profile->NormalsTexture = SetTexture2(PreviewComponent->GetWorld(), PackagePath,
					Profile->GetName() + "_Normals", Profile->NormalsTexture,
					TextureWidth_Vert, TextureHeight_Vert,
					Data,
					Profile->GetMaskedFlags() | RF_Public | RF_Standalone);

				Profile->NormalsTexture->Filter = TextureFilter::TF_Nearest;
				Profile->NormalsTexture->NeverStream = true;
				Profile->NormalsTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
				Profile->NormalsTexture->SRGB = false;
				Profile->NormalsTexture->Modify();
				Profile->NormalsTexture->MarkPackageDirty();
				Profile->NormalsTexture->PostEditChange();
				Profile->NormalsTexture->UpdateResource();
			}


			{
				EncodeData_Vec(VertPos, Profile->MaxValueOffset_Vert, true, Data);

				Profile->OffsetsTexture = SetTexture2(PreviewComponent->GetWorld(), PackagePath,
					Profile->GetName() + "_Offsets", Profile->OffsetsTexture,
					TextureWidth_Vert, TextureHeight_Vert,
					Data,
					Profile->GetMaskedFlags() | RF_Public | RF_Standalone);

				Profile->OffsetsTexture->Filter = TextureFilter::TF_Nearest;
				Profile->OffsetsTexture->NeverStream = true;
				Profile->OffsetsTexture->CompressionSettings = TextureCompressionSettings::TC_HDR;
				Profile->OffsetsTexture->SRGB = false;
				Profile->OffsetsTexture->Modify();
				Profile->OffsetsTexture->MarkPackageDirty();
				Profile->OffsetsTexture->PostEditChange();
				Profile->OffsetsTexture->UpdateResource();
			}
		
		}

		// Bone Textures
		if (Profile->Anims_Bone.Num())
		{
			TArray <FFloat16Color> Data;
			Data.SetNumZeroed(TextureWidth_Bone*TextureHeight_Bone);

			
			{
				EncodeData_Quat(true, BoneRot, Data);

				Profile->BoneRotTexture = SetTexture2(PreviewComponent->GetWorld(), PackagePath, 
					Profile->GetName() + "_BoneRot", Profile->BoneRotTexture,
					TextureWidth_Bone, TextureHeight_Bone, 
					Data,
					Profile->GetMaskedFlags() | RF_Public | RF_Standalone);

				Profile->BoneRotTexture->Filter = TextureFilter::TF_Nearest;
				Profile->BoneRotTexture->NeverStream = true;
				Profile->BoneRotTexture->CompressionSettings = TextureCompressionSettings::TC_HDR;
				Profile->BoneRotTexture->SRGB = false;
				Profile->BoneRotTexture->Modify();
				Profile->BoneRotTexture->MarkPackageDirty();
				Profile->BoneRotTexture->PostEditChange();
				Profile->BoneRotTexture->UpdateResource();
			}

			{
				EncodeData_Vec(BonePos, Profile->MaxValuePosition_Bone, true, Data);

				Profile->BonePosTexture = SetTexture2(PreviewComponent->GetWorld(), PackagePath,
					Profile->GetName() + "_BonePos", Profile->BonePosTexture,
					TextureWidth_Bone, TextureHeight_Bone, 
					Data,//BonePos,
					Profile->GetMaskedFlags() | RF_Public | RF_Standalone);

				Profile->BonePosTexture->Filter = TextureFilter::TF_Nearest;
				Profile->BonePosTexture->NeverStream = true;
				Profile->BonePosTexture->CompressionSettings = TextureCompressionSettings::TC_HDR;
				Profile->BonePosTexture->SRGB = false;

				Profile->BonePosTexture->Modify();
				Profile->BonePosTexture->MarkPackageDirty();
				Profile->BonePosTexture->PostEditChange();
				Profile->BonePosTexture->UpdateResource();
			}

		}

	}
}

void FVATEditorUtils::UVChannelsToSkeletalMesh(USkeletalMesh* Skel, const int32 LODIndex, const int32 UVChannelStart, TArray<TArray<FVector2D>>& UVChannels)
{
	check((UVChannelStart + UVChannels.Num()) <= MAX_TEXCOORDS);
	check(UVChannelStart < (int32)Skel->GetImportedModel()->LODModels[LODIndex].NumTexCoords);

	Skel->GetImportedModel()->LODModels[LODIndex].NumTexCoords = UVChannelStart + UVChannels.Num();

	const int32 Num = Skel->GetImportedModel()->LODModels[LODIndex].NumVertices;
	for (int32 j = 0; j < UVChannels.Num(); j++)
	{
		check(UVChannels[j].Num() == Num);
		for (int32 i = 0; i < Num; i++)
		{
			int32 SectionIndex = 0;
			int32 SoftIndex = 0;
			Skel->GetImportedModel()->LODModels[LODIndex].GetSectionFromVertexIndex(i, SectionIndex, SoftIndex);

			Skel->GetImportedModel()->LODModels[LODIndex].Sections[SectionIndex].SoftVertices[SoftIndex].UVs[UVChannelStart+j] = FVector2f(UVChannels[j][i]);
		}
	}
}


void FVATEditorUtils::IntoIslands(const TArray<int32>& IndexBuffer, const TArray<FVector2D>& UVs, TArray<int32>& OutIslandIDs, int32& OutNumIslands)
{
	
	TArray<FVector2D> UniqueUVs;
	TArray <TArray <int32>> PerVertTris;
	TArray<int32> NewIndexBuffer = IndexBuffer;
	
	for (int32 i = 0; i < IndexBuffer.Num(); i+=3)
	{
		const int32 A = IndexBuffer[i];
		const int32 B = IndexBuffer[i + 1];
		const int32 C = IndexBuffer[i + 2];

		int32 NewA = INDEX_NONE;
		int32 NewB = INDEX_NONE;
		int32 NewC = INDEX_NONE;

		for (int32 j = 0; j < UniqueUVs.Num(); j++)
		{
			if (UVs[A].Equals(UniqueUVs[j], FLOAT_NORMAL_THRESH)) NewA = j;
			if (UVs[B].Equals(UniqueUVs[j], FLOAT_NORMAL_THRESH)) NewB = j;
			if (UVs[C].Equals(UniqueUVs[j], FLOAT_NORMAL_THRESH)) NewC = j;
		}

		if (NewA == INDEX_NONE)
		{
			NewA = UniqueUVs.Add(UVs[A]);
			PerVertTris.AddZeroed(1);
		}

		if (NewB == INDEX_NONE)
		{
			NewB = UniqueUVs.Add(UVs[B]);
			PerVertTris.AddZeroed(1);
		}

		if (NewC == INDEX_NONE)
		{
			NewC = UniqueUVs.Add(UVs[C]);
			PerVertTris.AddZeroed(1);
		}

		NewIndexBuffer[i] = NewA;
		NewIndexBuffer[i + 1] = NewB;
		NewIndexBuffer[i + 2] = NewC;

		PerVertTris[NewA].AddUnique(i / 3);
		PerVertTris[NewB].AddUnique(i / 3);
		PerVertTris[NewC].AddUnique(i / 3);
	}
	
	TArray<int32> PerTriIsland;
	PerTriIsland.Init(INDEX_NONE, NewIndexBuffer.Num() / 3);
	
	int32 NumTris = NewIndexBuffer.Num() / 3;

	int32 NumIslands = 0;

	while (true)
	{
		int32 Winner = INDEX_NONE;
		for (int32 i = 0; i < NumTris; i++)
		{
			if (PerTriIsland[i] == INDEX_NONE)
			{
				Winner = i;
				break;
			}
		}


		if (Winner == INDEX_NONE) break;

		PerTriIsland[Winner] = NumIslands;
		NumIslands++;

		TArray <int32> CurrNeigs = { Winner };

		while (true)
		{
			if (CurrNeigs.Num() == 0) break;

			TArray <int32> NextNeigs;
			for (int32 n = 0; n < CurrNeigs.Num(); n++)
			{
				const int32 CurrN = CurrNeigs[n];
				for (int32 i = 0; i < 3; i++)
				{
					const auto& NeigTris = PerVertTris[NewIndexBuffer[CurrN * 3 + i]];
					for (int32 j = 0; j < NeigTris.Num(); j++)
					{
						if (PerTriIsland[NeigTris[j]] == INDEX_NONE)
						{
							PerTriIsland[NeigTris[j]] = PerTriIsland[CurrN];
							NextNeigs.Add(NeigTris[j]);
						}
					}
				}
			}

			CurrNeigs = NextNeigs;
		}
	}
	
	OutIslandIDs = PerTriIsland;
	OutNumIslands = NumIslands;
	
}

void FVATEditorUtils::ClosestUVPivotAssign(
	const TArray<int32>& IndexBuffer, const TArray<FVector2D>& UVs, const TArray<FVector2D>& PivotUVPos, TArray<int32>& VertPivotIDs)
{
	TArray<int32> IslandIDs;
	int32 NumIslands;
	IntoIslands(IndexBuffer, UVs, IslandIDs, NumIslands);

	TArray <int32> PerPivotIsland;
	PerPivotIsland.Init(INDEX_NONE, PivotUVPos.Num());

	for (int32 i = 0; i < IndexBuffer.Num(); i+=3)
	{
		FVector2D A = UVs[IndexBuffer[i]];
		FVector2D B = UVs[IndexBuffer[i+1]];
		FVector2D C = UVs[IndexBuffer[i+2]];

		FVector VA = FVector(A.X, A.Y, 0.0);
		FVector VB = FVector(B.X, B.Y, 0.0);
		FVector VC = FVector(C.X, C.Y, 0.0);

		for (int32 j = 0; j < PivotUVPos.Num(); j++)
		{
			const FVector BaryCentric = FMath::GetBaryCentric2D(FVector(PivotUVPos[j].X, PivotUVPos[j].Y, 0.0), VA, VB, VC);
			if (BaryCentric.X > 0.0f && BaryCentric.Y > 0.0f && BaryCentric.Z > 0.0f)
			{
				PerPivotIsland[j] = IslandIDs[i / 3];
			}
		}
	}

	VertPivotIDs.SetNum(UVs.Num());

	for (int32 i = 0; i < IndexBuffer.Num(); i += 3)
	{
		int32 IslandID = IslandIDs[i / 3];
		FVector2D A = UVs[IndexBuffer[i]];
		FVector2D B = UVs[IndexBuffer[i + 1]];
		FVector2D C = UVs[IndexBuffer[i + 2]];

		float LA = MAX_FLT, LB = MAX_FLT, LC = MAX_FLT;
		int32 WA = INDEX_NONE, WB = INDEX_NONE, WC = INDEX_NONE;

		for (int32 j = 0; j < PivotUVPos.Num(); j++)
		{
			if (PerPivotIsland[j] == IslandID)
			{
				float DA = FVector2D::Distance(A, PivotUVPos[j]);
				float DB = FVector2D::Distance(B, PivotUVPos[j]);
				float DC = FVector2D::Distance(C, PivotUVPos[j]);

				if (DA < LA)
				{
					LA = DA;
					WA = j;
				}

				if (DB < LB)
				{
					LB = DB;
					WB = j;
				}

				if (DC < LC)
				{
					LC = DC;
					WC = j;
				}
			}
		}

		VertPivotIDs[IndexBuffer[i]] = WA;
		VertPivotIDs[IndexBuffer[i+1]] = WB;
		VertPivotIDs[IndexBuffer[i+2]] = WC;
	}
}


#undef LOCTEXT_NAMESPACE

