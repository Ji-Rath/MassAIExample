// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimToTextureBPLibrary.h"
#include "AnimToTexture.h"
#include "AnimToTextureUtils.h"
#include "AnimToTextureSkeletalMesh.h"
#include "AnimToTextureDataAsset.h"
#include "EvaluateSequenceAnimInstance.h"

#include "Editor.h"
#include "LevelEditor.h"
#include "RawMesh.h"
#include "MeshUtilities.h"
#include "AssetRegistryModule.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Math/Vector.h"
#include "Math/NumericLimits.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MeshDescription.h"
#include "Materials/MaterialInstanceConstant.h"
#include "MaterialEditingLibrary.h"
#include "EditorSupportDelegates.h"

UAnimToTextureBPLibrary::UAnimToTextureBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UAnimToTextureBPLibrary::AnimationToTexture(UAnimToTextureDataAsset* DataAsset,
	const FTransform RootTransform, const bool bCreateTextures, const bool bCreateUVChannel)
{
	if (!DataAsset)
	{
		return;
	}

	// Reset DataAsset Values
	DataAsset->Reset();

	if (!DataAsset->GetSkeletalMesh() || !DataAsset->GetStaticMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Meshes"));
		return;
	}

	// Check LODs
	if (!DataAsset->GetSkeletalMesh()->IsValidLODIndex(DataAsset->SkeletalLODIndex) || 
		DataAsset->StaticLODIndex >= DataAsset->GetStaticMesh()->GetNumLODs())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid LOD Indices"));
		return;
	}

	// Check Socket.
	bool bValidSocket = false;
	if (DataAsset->AttachToSocket.IsValid() && !DataAsset->AttachToSocket.IsNone())
	{
		if (AnimToTexture_Private::HasBone(DataAsset->GetSkeletalMesh(), DataAsset->AttachToSocket))
		{
			bValidSocket = true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Socket: %s"), *DataAsset->AttachToSocket.ToString());
			return;
		}
	}
	
	if (bValidSocket && DataAsset->Mode == EAnimToTextureMode::Vertex)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to use Socket in Vertex Mode. Use Bone Mode instead."));
		return;
	}

	// ---------------------------------------------------------------------------		
	// Get Meshes Vertices and Mapping.
	// NOTE: We need to create a Mapping between the StaticMesh|MeshDescription and the SkeletalMesh
	//       Since they dont have same number of points.
	//	
	TArray<FVector3f> Vertices;
	TArray<FVector3f> SkelVertices;
	TArray<int32> StaticToSkelMapping;

	AnimToTexture_Private::GetStaticToSkeletalMapping(
		*DataAsset->GetStaticMesh(), DataAsset->StaticLODIndex,
		*DataAsset->GetSkeletalMesh(), DataAsset->SkeletalLODIndex,
		Vertices, SkelVertices,
		StaticToSkelMapping);
	
	// Set Static Number of Vertices 
	// NOTE: these are the MeshDescription Vertices.
	const int32 NumVertices = Vertices.Num();

	// --------------------------------------------------------------------------

	// Create Temp Actor
	UWorld* World = GEditor->GetEditorWorldContext().World();
	AActor* Actor = World->SpawnActor<AActor>();

	// Create Temp MasterMeshComponent
	USkeletalMeshComponent* MasterMeshComponent = nullptr;
	if (DataAsset->GetMasterSkeletalMesh())
	{
		MasterMeshComponent = NewObject<USkeletalMeshComponent>(Actor);
		MasterMeshComponent->SetSkeletalMesh(DataAsset->GetMasterSkeletalMesh());
		MasterMeshComponent->SetForcedLOD(1); // Force to LOD0
		MasterMeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		MasterMeshComponent->SetUpdateAnimationInEditor(true);
		MasterMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;		
		MasterMeshComponent->RegisterComponent();
	};

	// Create Temp SkeletalMesh Component
	USkeletalMeshComponent* MeshComponent = NewObject<USkeletalMeshComponent>(Actor);
	MeshComponent->SetSkeletalMesh(DataAsset->GetSkeletalMesh());
	MeshComponent->SetForcedLOD(1); // Force to LOD0;
	MeshComponent->RegisterComponent();
	
	USkeletalMeshComponent* BaseMeshComponent = nullptr;
	if (MasterMeshComponent)
	{
		MeshComponent->SetupAttachment(MasterMeshComponent);
		MeshComponent->SetMasterPoseComponent(MasterMeshComponent);

		BaseMeshComponent = MasterMeshComponent;
	}
	else
	{
		MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		MeshComponent->SetUpdateAnimationInEditor(true);
		MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		
		BaseMeshComponent = MeshComponent;
	}
	
	// --------------------------------------------------------------------------
	// Set AnimBlueprint
	UEvaluateSequenceAnimInstance* EvaluationAnimInstance = nullptr;
	if (DataAsset->AnimInstanceClass != nullptr)
	{
		// Set Anim Blueprint
		BaseMeshComponent->SetAnimInstanceClass(DataAsset->AnimInstanceClass);
		EvaluationAnimInstance = Cast<UEvaluateSequenceAnimInstance>(BaseMeshComponent->GetAnimInstance());
	}

	// Get Mapping between Slave and Master
	TArray<int32> SlaveBoneMap;
	if (DataAsset->GetMasterSkeletalMesh() &&
		DataAsset->Mode == EAnimToTextureMode::Bone)
	{
		AnimToTexture_Private::GetSlaveBoneMap(DataAsset->GetMasterSkeletalMesh(), DataAsset->GetSkeletalMesh(), SlaveBoneMap);
	}
	
	// ---------------------------------------------------------------------------
	// Get Reference Skeleton Transforms
	//
	int32 NumBones = INDEX_NONE;
	int32 SocketIndex = INDEX_NONE;
	TArray<FName>     BoneNames;
	TArray<FVector3f> BoneRefPositions;
	TArray<FVector4>  BoneRefRotations;
	TArray<FVector3f> BonePositions;
	TArray<FVector4>  BoneRotations;
	
	if (DataAsset->Mode == EAnimToTextureMode::Bone)
	{
		// Get Number of RawBones (no virtual)
		NumBones = AnimToTexture_Private::GetNumBones(BaseMeshComponent->SkeletalMesh);

		// Get Raw Ref Bone (no virtual)
		TArray<FTransform> RefBoneTransforms;
		AnimToTexture_Private::GetRefBoneTransforms(BaseMeshComponent->SkeletalMesh, RefBoneTransforms);
		AnimToTexture_Private::DecomposeTransformations(RefBoneTransforms, BoneRefPositions, BoneRefRotations);

		// Get Bone Names (no virtual)
		AnimToTexture_Private::GetBoneNames(BaseMeshComponent->SkeletalMesh, BoneNames);

		// Make sure array sizes are correct.
		check(RefBoneTransforms.Num() == NumBones && BoneNames.Num() == NumBones);

		// Check if Socket is in BoneNames
		if (bValidSocket && !BoneNames.Find(DataAsset->AttachToSocket, SocketIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("Socket: %s not found in Raw Bone List"), *DataAsset->AttachToSocket.ToString());
			return;
		}

		// Add RefPose 
		// Note: this is added in the first frame of the Bone Position and Rotation Textures
		BonePositions.Append(BoneRefPositions);
		BoneRotations.Append(BoneRefRotations);
	}

	// ---------------------------------------------------------------------------
	// Get Vertex Data (for all frames)
	//		
	TArray<FVector3f> VertexDeltas;
	TArray<FVector3f> VertexNormals;
	
	// Get Animation Frames Data
	//
	const float SampleInterval = 1.f / DataAsset->SampleRate;

	for (const FAnimSequenceInfo& AnimSequenceInfo : DataAsset->AnimSequences)
	{
		UAnimSequence* AnimSequence = AnimSequenceInfo.AnimSequence;

		if (!AnimSequenceInfo.bEnabled || !AnimSequence)
		{
			continue;
		}
		
		// Make sure SkelMesh and AnimSequence use same Skeleton
		if (AnimSequence->GetSkeleton() != BaseMeshComponent->SkeletalMesh->GetSkeleton())
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid AnimSequence: %s for given SkeletalMesh: %s"), *AnimSequence->GetFName().ToString(), *BaseMeshComponent->SkeletalMesh->GetFName().ToString());
			continue;
		}
		// Set AnimSequence
		else
		{
			if (IsValid(EvaluationAnimInstance))
			{
				EvaluationAnimInstance->SequenceToEvaluate = AnimSequence;
				EvaluationAnimInstance->TimeToEvaluate = 0.0f;
			}
			else
			{
				BaseMeshComponent->SetAnimation(AnimSequence);
			}
		}

		// -----------------------------------------------------------------------------------
		// Get Number of Frames
		//
		int32 StartFrame;
		int32 EndFrame;
		
		// Get Range from AnimSequence
		if (!AnimSequenceInfo.bUseCustomRange)
		{
			StartFrame = 0;
			EndFrame = AnimSequence->GetNumberOfSampledKeys() - 1; // AnimSequence->GetNumberOfFrames();
		}
		// Get Range from DataAsset
		else
		{
			StartFrame = AnimSequenceInfo.StartFrame;
			EndFrame = AnimSequenceInfo.EndFrame;
		}
		if (EndFrame - StartFrame <= 0)
		{
			continue;
		}
		
		// ---------------------------------------------------------------------------
		// 
		float Time = 0.f;
		float EndTime = AnimSequence->GetTimeAtFrame(EndFrame);

		int32 SampleIndex = 0;

		while (Time < EndTime)
		{
			Time = FMath::Clamp(SampleIndex * SampleInterval, 0.f, EndTime);
			SampleIndex++;

			// Go To Time
			if (IsValid(EvaluationAnimInstance))
			{
				EvaluationAnimInstance->TimeToEvaluate = Time;
			}
			else
			{
				BaseMeshComponent->SetPosition(Time);
			}

			// Update SkelMesh Animation.
			BaseMeshComponent->TickAnimation(0.0f, false /*bNeedsValidRootMotion*/);
			// BaseMeshComponent->TickComponent(0.0f, ELevelTick::LEVELTICK_All, nullptr);
			BaseMeshComponent->RefreshBoneTransforms(nullptr /*TickFunction*/);
			BaseMeshComponent->RefreshSlaveComponents();
			
			// ---------------------------------------------------------------------------
			// Store Vertex Deltas & Normals.
			//
			if (DataAsset->Mode == EAnimToTextureMode::Vertex)
			{
				TArray<FVector3f> Positions;
				TArray<FVector3f> Normals;
				AnimToTexture_Private::GetSkinnedVertices(MeshComponent, DataAsset->SkeletalLODIndex,
					Positions, Normals);

				// Loop thru static vertices and find the mapped SkeletalMesh Vertex
				for (int32 VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
				{
					const int32 SkelVertexIndex = StaticToSkelMapping[VertexIndex];

					// Delta & Normal
					const FVector3f VertexDelta = (FVector3f)RootTransform.TransformPosition((FVector)Positions[SkelVertexIndex]) - SkelVertices[SkelVertexIndex];
					const FVector3f VertexNormal = (FVector3f)RootTransform.TransformVector((FVector)Normals[SkelVertexIndex]);

					VertexDeltas.Add(VertexDelta);
					VertexNormals.Add(VertexNormal);
				}
			} // End Vertex Mode

			// ---------------------------------------------------------------------------
			// Store Bone Positions & Rotations
			//
			else if (DataAsset->Mode == EAnimToTextureMode::Bone)
			{
				// Get Relative Transforms
				// Note: Size is of Raw bones in SkeletalMesh. These are the original/raw bones of the asset, without Virtual Bones.
				// Note: You cannot call CacheRefToLocalMatrices on a SlaveComponent
				TArray<FMatrix44f> RefToLocals;
				BaseMeshComponent->CacheRefToLocalMatrices(RefToLocals);
				
				// check size
				check(RefToLocals.Num() == NumBones);

				// Get Component Space Transforms
				// Note returns all transforms, including VirtualBones
				// Note: You cannot call GetComponentSpaceTransforms on a SlaveComponent
				const TArray<FTransform>& CompSpaceTransforms = BaseMeshComponent->GetComponentSpaceTransforms();
				check(CompSpaceTransforms.Num() >= RefToLocals.Num());

				for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
				{
					// Decompose Transformation (ComponentSpace)
					const FTransform& CompSpaceTransform = CompSpaceTransforms[BoneIndex];
					FVector3f BonePosition;
					FVector4 BoneRotation;
					AnimToTexture_Private::DecomposeTransformation(CompSpaceTransform, BonePosition, BoneRotation);

					// Position Delta (from RefPose)
					const FVector3f Delta = BonePosition - BoneRefPositions[BoneIndex];

					// Decompose Transformation (Relative to RefPose)
					FVector3f BoneRelativePosition;
					FVector4 BoneRelativeRotation;
					FMatrix RefToLocalMatrix = FMatrix(RefToLocals[BoneIndex]);
					const FTransform RelativeTransform(RefToLocalMatrix);
					AnimToTexture_Private::DecomposeTransformation(RelativeTransform, BoneRelativePosition, BoneRelativeRotation);

					BonePositions.Add(Delta);
					BoneRotations.Add(BoneRelativeRotation);
				}
			} // End Bone Mode
		} // End Frame

		// Store Anim Info Data
		FAnimInfo AnimInfo;
		AnimInfo.NumFrames = SampleIndex;
		AnimInfo.AnimStart = DataAsset->NumFrames;
		AnimInfo.bLooping = AnimSequenceInfo.bLooping;
		DataAsset->Animations.Add(AnimInfo);

		// Accumulate Frames
		DataAsset->NumFrames += AnimInfo.NumFrames;
	} // End Anim
		
	// Destroy Temp Component
	MeshComponent->UnregisterComponent();
	MeshComponent->DestroyComponent();

	// Destroy Master Component
	if (MasterMeshComponent)
	{
		MasterMeshComponent->UnregisterComponent();
		MasterMeshComponent->DestroyComponent();
	}
	
	Actor->Destroy();
	
	// ---------------------------------------------------------------------------
	// Nothing to do here ...
	//
	if (!DataAsset->NumFrames || !NumVertices)
	{
		return;
	}

	// ---------------------------------------------------------------------------
	if (DataAsset->Mode == EAnimToTextureMode::Vertex)
	{
		// Find Best Resolution for Vertex Data
		int32 Height, Width;
		if (!FindBestResolution(DataAsset->NumFrames, NumVertices, 
								Height, Width, DataAsset->VertexRowsPerFrame, 
								DataAsset->MaxHeight, DataAsset->MaxWidth, DataAsset->bEnforcePowerOfTwo))
		{
			UE_LOG(LogTemp, Warning, TEXT("Vertex Animation data cannot be fit in a %ix%i texture."), DataAsset->MaxHeight, DataAsset->MaxWidth);
			return;
		}

		// Normalize Vertex Data
		TArray<FVector3f> NormalizedVertexDeltas;
		TArray<FVector3f> NormalizedVertexNormals;
		NormalizeVertexData(
			VertexDeltas, VertexNormals,
			DataAsset->VertexMinBBox, DataAsset->VertexSizeBBox,
			NormalizedVertexDeltas, NormalizedVertexNormals);

		// Write Textures
		if (bCreateTextures)
		{
			AnimToTexture_Private::WriteVectorsToTexture<FVector3f, AnimToTexture_Private::FLowPrecision>(NormalizedVertexDeltas, DataAsset->NumFrames, DataAsset->VertexRowsPerFrame, Height, Width, DataAsset->GetVertexPositionTexture());
			AnimToTexture_Private::WriteVectorsToTexture<FVector3f, AnimToTexture_Private::FLowPrecision>(NormalizedVertexNormals, DataAsset->NumFrames, DataAsset->VertexRowsPerFrame, Height, Width, DataAsset->GetVertexNormalTexture());
		}

		// Add Vertex UVChannel
		if (bCreateUVChannel)
		{
			CreateUVChannel(DataAsset->GetStaticMesh(), DataAsset->StaticLODIndex, DataAsset->UVChannel,
				Height, Width);
		}
	}

	// ---------------------------------------------------------------------------
	
	if (DataAsset->Mode == EAnimToTextureMode::Bone)
	{
		// Find Best Resolution for Bone Data
		int32 Height, Width;

		// NOTE: If NumBones are > 256, you might need to use MasterPose

		// Note we are adding +1 frame for the ref pose
		if (!FindBestResolution(DataAsset->NumFrames + 1, NumBones, 
								Height, Width, DataAsset->BoneRowsPerFrame, 
								DataAsset->MaxHeight, DataAsset->MaxWidth, DataAsset->bEnforcePowerOfTwo))
		{
			UE_LOG(LogTemp, Warning, TEXT("Bone Animation data cannot be fit in a %ix%i texture."), DataAsset->MaxHeight, DataAsset->MaxWidth);
			return;
		}

		// Write Bone Position and Rotation Textures
		if (bCreateTextures)
		{
			// Normalize Bone Data
			TArray<FVector3f> NormalizedBonePositions;
			TArray<FVector4> NormalizedBoneRotations;
			NormalizeBoneData(
				BonePositions, BoneRotations,
				DataAsset->BoneMinBBox, DataAsset->BoneSizeBBox,
				NormalizedBonePositions, NormalizedBoneRotations);

			// Write Textures
			if (DataAsset->PositionAndRotationPrecision == EAnimToTextureBonePrecision::SixteenBits)
			{
				AnimToTexture_Private::WriteVectorsToTexture<FVector3f, AnimToTexture_Private::FHighPrecision>(NormalizedBonePositions, DataAsset->NumFrames + 1, DataAsset->BoneRowsPerFrame, Height, Width, DataAsset->GetBonePositionTexture());
				AnimToTexture_Private::WriteVectorsToTexture<FVector4, AnimToTexture_Private::FHighPrecision>(NormalizedBoneRotations, DataAsset->NumFrames + 1, DataAsset->BoneRowsPerFrame, Height, Width, DataAsset->GetBoneRotationTexture());
			}
			else
			{
				AnimToTexture_Private::WriteVectorsToTexture<FVector3f, AnimToTexture_Private::FLowPrecision>(NormalizedBonePositions, DataAsset->NumFrames + 1, DataAsset->BoneRowsPerFrame, Height, Width, DataAsset->GetBonePositionTexture());
				AnimToTexture_Private::WriteVectorsToTexture<FVector4, AnimToTexture_Private::FLowPrecision>(NormalizedBoneRotations, DataAsset->NumFrames + 1, DataAsset->BoneRowsPerFrame, Height, Width, DataAsset->GetBoneRotationTexture());
			}
		}

		// ---------------------------------------------------------------------------

		// Find Best Resolution for Weights Data
		if (!FindBestResolution(2, NumVertices, 
								Height, Width, DataAsset->BoneWeightRowsPerFrame, 
								DataAsset->MaxHeight, DataAsset->MaxWidth, DataAsset->bEnforcePowerOfTwo))
		{
			UE_LOG(LogTemp, Warning, TEXT("Weights Data cannot be fit in a %ix%i texture."), DataAsset->MaxHeight, DataAsset->MaxWidth);
			return;
		}

		// Write Weights Texture
		if (bCreateTextures)
		{
			TArray<AnimToTexture_Private::TVertexSkinWeight<4>> SkinWeights;

			// Store Influence Weights
			if (!bValidSocket)
			{
				// Get SkinWeights mapped to StaticMesh
				AnimToTexture_Private::GetReducedSkinWeightsData(
					*DataAsset->GetSkeletalMesh(), DataAsset->SkeletalLODIndex,
					StaticToSkelMapping, 
					SkinWeights);

				// Remap MeshBoneIndices to SlaveBoneMap
				if (DataAsset->GetMasterSkeletalMesh())
				{
					for (AnimToTexture_Private::TVertexSkinWeight<4>& SkinWeight : SkinWeights)
					{
						SkinWeight.MeshBoneIndices[0] = SlaveBoneMap[SkinWeight.MeshBoneIndices[0]];
						SkinWeight.MeshBoneIndices[1] = SlaveBoneMap[SkinWeight.MeshBoneIndices[1]];
						SkinWeight.MeshBoneIndices[2] = SlaveBoneMap[SkinWeight.MeshBoneIndices[2]];
						SkinWeight.MeshBoneIndices[3] = SlaveBoneMap[SkinWeight.MeshBoneIndices[3]];
					}
				}
			}
			// If Valid Socket, set all influences to same index.
			else
			{
				// Set all indices and weights to same SocketIndex
				SkinWeights.SetNumUninitialized(NumVertices);
				for (AnimToTexture_Private::TVertexSkinWeight<4>& SkinWeight : SkinWeights)
				{
					SkinWeight.BoneWeights = TStaticArray<uint8, 4>(InPlace, 255);
					SkinWeight.MeshBoneIndices = TStaticArray<uint16, 4>(InPlace, SocketIndex);
				}
			}

			// Write Bone Weights Texture
			AnimToTexture_Private::WriteSkinWeightsToTexture(SkinWeights,
				DataAsset->BoneWeightRowsPerFrame, Height, Width, DataAsset->GetBoneWeightTexture());
		}

		// Add Vertex UVChannel
		if (bCreateUVChannel)
		{
			CreateUVChannel(DataAsset->GetStaticMesh(), DataAsset->StaticLODIndex, DataAsset->UVChannel,
				Height, Width);
		}
	}

	// ---------------------------------------------------------------------------
	// Mark DataAsset as Dirty.
	//
	DataAsset->MarkPackageDirty();
}


void UAnimToTextureBPLibrary::UpdateMaterialInstanceFromDataAsset(UAnimToTextureDataAsset* DataAsset, UMaterialInstanceConstant* MaterialInstance, 
	const FAnimToTextureMaterialParamNames& ParamNames, const EMaterialParameterAssociation MaterialParameterAssociation)
{
	if (!MaterialInstance || !DataAsset)
	{
		return;
	}
	
	// Set Preview Mesh
	if (DataAsset->GetStaticMesh())
	{
		MaterialInstance->PreviewMesh = DataAsset->GetStaticMesh();
	}

	if (DataAsset->Mode == EAnimToTextureMode::Vertex)
	{
		FLinearColor VectorParameter;
		VectorParameter = FLinearColor(DataAsset->VertexMinBBox);
		UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(MaterialInstance, ParamNames.BoundingBoxMin, VectorParameter, MaterialParameterAssociation);
		
		VectorParameter = FLinearColor(DataAsset->VertexSizeBBox);
		UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(MaterialInstance, ParamNames.BoundingBoxScale, VectorParameter, MaterialParameterAssociation);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(MaterialInstance, ParamNames.NumFrames, DataAsset->NumFrames, MaterialParameterAssociation);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(MaterialInstance, ParamNames.RowsPerFrame, DataAsset->VertexRowsPerFrame, MaterialParameterAssociation);

		UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(MaterialInstance, ParamNames.VertexPositionTexture, DataAsset->GetVertexPositionTexture(), MaterialParameterAssociation);
		UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(MaterialInstance, ParamNames.VertexNormalTexture, DataAsset->GetVertexNormalTexture(), MaterialParameterAssociation);

	}
	else if (DataAsset->Mode == EAnimToTextureMode::Bone)
	{
		FLinearColor VectorParameter;
		VectorParameter = FLinearColor(DataAsset->BoneMinBBox);
		UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(MaterialInstance, ParamNames.BoundingBoxMin, VectorParameter, MaterialParameterAssociation);

		VectorParameter = FLinearColor(DataAsset->BoneSizeBBox);
		UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(MaterialInstance, ParamNames.BoundingBoxScale, VectorParameter, MaterialParameterAssociation);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(MaterialInstance, ParamNames.NumFrames, DataAsset->NumFrames, MaterialParameterAssociation);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(MaterialInstance, ParamNames.RowsPerFrame, DataAsset->BoneRowsPerFrame, MaterialParameterAssociation);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(MaterialInstance, ParamNames.BoneWeightRowsPerFrame, DataAsset->BoneWeightRowsPerFrame, MaterialParameterAssociation);

		UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(MaterialInstance, ParamNames.BonePositionTexture, DataAsset->GetBonePositionTexture(), MaterialParameterAssociation);
		UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(MaterialInstance, ParamNames.BoneRotationTexture, DataAsset->GetBoneRotationTexture(), MaterialParameterAssociation);
		UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(MaterialInstance, ParamNames.BoneWeightsTexture, DataAsset->GetBoneWeightTexture(), MaterialParameterAssociation);
	}

	// Update Material
	UMaterialEditingLibrary::UpdateMaterialInstance(MaterialInstance);

	// Rebuild Material
	UMaterialEditingLibrary::RebuildMaterialInstanceEditors(MaterialInstance->GetMaterial());
}

void UAnimToTextureBPLibrary::UpdateMaterialLayerFunction(
	UMaterialInstanceConstant* MaterialInstance,
	UMaterialFunctionInterface* OldMaterialFunction,
	UMaterialFunctionInterface* NewMaterialFunction)
{
	if (!IsValid(MaterialInstance))
	{
		return;
	}

	const FMaterialParameterInfo ParameterInfo(TEXT("BoneAnimation"), EMaterialParameterAssociation::GlobalParameter);

	FMaterialLayersFunctions LayersValue;
	FGuid TempGuid(0, 0, 0, 0);

	// TODO: Ben.Ingram / Cesar.Castro verify this.
	bool bMaterialLayerReplaced = false;
	if (MaterialInstance->GetMaterialLayers(LayersValue))
	{
		for (int32 LayerIdx = 0; LayerIdx < LayersValue.Layers.Num(); ++LayerIdx)
		{
			TObjectPtr<UMaterialFunctionInterface>& MaterialFunction = LayersValue.Layers[LayerIdx];
			if (MaterialFunction.Get() == OldMaterialFunction)
			{
				MaterialFunction = NewMaterialFunction;
				LayersValue.LayerLinkStates[LayerIdx] = EMaterialLayerLinkState::UnlinkedFromParent;
				bMaterialLayerReplaced = true;
				break;
			}

		}
	}

	if (bMaterialLayerReplaced)
	{
		// TODO: Ben.Ingram / Cesar.Castro verify this.
		MaterialInstance->SetMaterialLayers(LayersValue);

		// Update Material
		UMaterialEditingLibrary::UpdateMaterialInstance(MaterialInstance);

		// Rebuild Material
		UMaterialEditingLibrary::RebuildMaterialInstanceEditors(MaterialInstance->GetMaterial());
	}
}


UStaticMesh* UAnimToTextureBPLibrary::ConvertSkeletalMeshToStaticMesh(USkeletalMesh* SkeletalMesh, const FString PackageName, const int32 LODIndex)
{
	if (!SkeletalMesh || PackageName.IsEmpty())
	{
		return nullptr;
	}

	if (!FPackageName::IsValidObjectPath(PackageName))
	{
		return nullptr;
	}

	if (LODIndex >= 0 && !SkeletalMesh->IsValidLODIndex(LODIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid LODIndex: %i"), LODIndex);
		return nullptr;
	}

	// Create Temp Actor
	UWorld* World = GEditor->GetEditorWorldContext().World();
	AActor* Actor = World->SpawnActor<AActor>();

	// Create Temp SkeletalMesh Component
	USkeletalMeshComponent* MeshComponent = NewObject<USkeletalMeshComponent>(Actor);
	MeshComponent->RegisterComponent();
	MeshComponent->SetSkeletalMesh(SkeletalMesh);
	TArray<UMeshComponent*> MeshComponents = { MeshComponent };

	UStaticMesh* OutStaticMesh = nullptr;
	bool bGeneratedCorrectly = true;

	// Create New StaticMesh
	if (!FPackageName::DoesPackageExist(PackageName))
	{
		IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
		OutStaticMesh = MeshUtilities.ConvertMeshesToStaticMesh(MeshComponents, FTransform::Identity, PackageName);
	}
	// Update Existing StaticMesh
	else
	{
		// Load Existing Mesh
		OutStaticMesh = LoadObject<UStaticMesh>(nullptr, *PackageName);
	}

	if (OutStaticMesh)
	{
		// Create Temp Package.
		// because 
		UPackage* TransientPackage = GetTransientPackage();

		// Create Temp Mesh.
		IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
		UStaticMesh* TempMesh = MeshUtilities.ConvertMeshesToStaticMesh(MeshComponents, FTransform::Identity, TransientPackage->GetPathName());

		// make sure transactional flag is on
		TempMesh->SetFlags(RF_Transactional);

		// Copy All LODs
		if (LODIndex < 0)
		{
			const int32 NumSourceModels = TempMesh->GetNumSourceModels();
			OutStaticMesh->SetNumSourceModels(NumSourceModels);

			for (int32 Index = 0; Index < NumSourceModels; ++Index)
			{
				// Get RawMesh
				FRawMesh RawMesh;
				TempMesh->GetSourceModel(Index).LoadRawMesh(RawMesh);

				// Set RawMesh
				OutStaticMesh->GetSourceModel(Index).SaveRawMesh(RawMesh);
			};
		}

		// Copy Single LOD
		else
		{
			if (LODIndex >= TempMesh->GetNumSourceModels())
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid Source Model Index: %i"), LODIndex);
				bGeneratedCorrectly = false;
			}
			else
			{
				OutStaticMesh->SetNumSourceModels(1);

				// Get RawMesh
				FRawMesh RawMesh;
				TempMesh->GetSourceModel(LODIndex).LoadRawMesh(RawMesh);

				// Set RawMesh
				OutStaticMesh->GetSourceModel(0).SaveRawMesh(RawMesh);
			}
		}
			
		// Copy Materials
		const TArray<FStaticMaterial>& Materials = TempMesh->GetStaticMaterials();
		OutStaticMesh->SetStaticMaterials(Materials);

		// Done
		TArray<FText> OutErrors;
		OutStaticMesh->Build(true, &OutErrors);
		OutStaticMesh->MarkPackageDirty();
	}

	// Destroy Temp Component and Actor
	MeshComponent->UnregisterComponent();
	MeshComponent->DestroyComponent();
	Actor->Destroy();

	return bGeneratedCorrectly ? OutStaticMesh : nullptr;
}


void UAnimToTextureBPLibrary::NormalizeVertexData(
	const TArray<FVector3f>& Deltas, const TArray<FVector3f>& Normals,
	FVector& MinBBox, FVector& SizeBBox,
	TArray<FVector3f>& NormalizedDeltas, TArray<FVector3f>& NormalizedNormals)
{
	check(Deltas.Num() == Normals.Num());

	// ---------------------------------------------------------------------------
	// Compute Bounding Box
	//
	MinBBox = { TNumericLimits<float>::Max(), TNumericLimits<float>::Max(), TNumericLimits<float>::Max() };
	FVector3f MaxBBox = { TNumericLimits<float>::Min(), TNumericLimits<float>::Min(), TNumericLimits<float>::Min() };
	
	for (const FVector3f& Delta: Deltas)
	{
		// Find Min/Max BoundingBox
		MinBBox.X = FMath::Min(Delta.X, MinBBox.X);
		MinBBox.Y = FMath::Min(Delta.Y, MinBBox.Y);
		MinBBox.Z = FMath::Min(Delta.Z, MinBBox.Z);

		MaxBBox.X = FMath::Max(Delta.X, MaxBBox.X);
		MaxBBox.Y = FMath::Max(Delta.Y, MaxBBox.Y);
		MaxBBox.Z = FMath::Max(Delta.Z, MaxBBox.Z);
	}

	SizeBBox = (FVector)MaxBBox - MinBBox;

	// ---------------------------------------------------------------------------
	// Normalize Vertex Position Deltas
	// Basically we want all deltas to be between [0, 1]
	
	// Compute Normalization Factor per-axis.
	const FVector NormFactor = {
		1.0f / static_cast<float>(SizeBBox.X),
		1.0f / static_cast<float>(SizeBBox.Y),
		1.0f / static_cast<float>(SizeBBox.Z) };

	NormalizedDeltas.SetNumUninitialized(Deltas.Num());
	for (int32 Index = 0; Index < Deltas.Num(); ++Index)
	{
		NormalizedDeltas[Index] = (FVector3f)(((FVector)Deltas[Index] - MinBBox) * NormFactor);
	}

	// ---------------------------------------------------------------------------
	// Normalize Vertex Normals
	// And move them to [0, 1]
	
	NormalizedNormals.SetNumUninitialized(Normals.Num());
	for (int32 Index = 0; Index < Normals.Num(); ++Index)
	{
		NormalizedNormals[Index] = (Normals[Index].GetSafeNormal() + FVector3f::OneVector) * 0.5f;
	}

}

void UAnimToTextureBPLibrary::NormalizeBoneData(
	const TArray<FVector3f>& Positions, const TArray<FVector4>& Rotations,
	FVector& MinBBox, FVector& SizeBBox, 
	TArray<FVector3f>& NormalizedPositions, TArray<FVector4>& NormalizedRotations)
{
	check(Positions.Num() == Rotations.Num());

	// ---------------------------------------------------------------------------
	// Compute Position Bounding Box
	//
	MinBBox = { TNumericLimits<float>::Max(), TNumericLimits<float>::Max(), TNumericLimits<float>::Max() };
	FVector3f MaxBBox = { TNumericLimits<float>::Min(), TNumericLimits<float>::Min(), TNumericLimits<float>::Min() };

	for (const FVector3f& Position : Positions)
	{
		// Find Min/Max BoundingBox
		MinBBox.X = FMath::Min(Position.X, MinBBox.X);
		MinBBox.Y = FMath::Min(Position.Y, MinBBox.Y);
		MinBBox.Z = FMath::Min(Position.Z, MinBBox.Z);

		MaxBBox.X = FMath::Max(Position.X, MaxBBox.X);
		MaxBBox.Y = FMath::Max(Position.Y, MaxBBox.Y);
		MaxBBox.Z = FMath::Max(Position.Z, MaxBBox.Z);
	}

	SizeBBox = (FVector)MaxBBox - MinBBox;

	// ---------------------------------------------------------------------------
	// Normalize Bone Position.
	// Basically we want all positions to be between [0, 1]

	// Compute Normalization Factor per-axis.
	const FVector NormFactor = {
		1.0f / static_cast<float>(SizeBBox.X),
		1.0f / static_cast<float>(SizeBBox.Y),
		1.0f / static_cast<float>(SizeBBox.Z) };

	NormalizedPositions.SetNumUninitialized(Positions.Num());
	for (int32 Index = 0; Index < Positions.Num(); ++Index)
	{
		NormalizedPositions[Index] = FVector3f(((FVector)Positions[Index] - MinBBox) * NormFactor);
	}

	// ---------------------------------------------------------------------------
	// Normalize Rotations
	// And move them to [0, 1]
	NormalizedRotations.SetNumUninitialized(Rotations.Num());
	for (int32 Index = 0; Index < Rotations.Num(); ++Index)
	{
		const FVector4 Axis = Rotations[Index];
		const float Angle = Rotations[Index].W; // Angle are returned in radians and they go from [0-pi*2]

		NormalizedRotations[Index] = (Axis.GetSafeNormal() + FVector::OneVector) * 0.5f;
		NormalizedRotations[Index].W = Angle / (PI * 2.f);
	}
}


bool UAnimToTextureBPLibrary::CreateUVChannel(
	UStaticMesh* StaticMesh, const int32 LODIndex, const int32 UVChannelIndex,
	const int32 Height, const int32 Width)
{
	if (!StaticMesh)
	{
		return false;
	}

	// Get Render Data
	const FStaticMeshRenderData* RenderData = StaticMesh->GetRenderData();
	check(RenderData);

	// Get LOD Data
	if (!RenderData->LODResources.IsValidIndex(LODIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid LODIndex: %i"), LODIndex);
		return false;
	}

	// ----------------------------------------------------------------------------
	// Get Mesh Description.
	// This is needed for Inserting UVChannel
	FMeshDescription* MeshDescription = StaticMesh->GetMeshDescription(LODIndex);
	check(MeshDescription);

	// Check if UVChannel is being used by the Lightmap UV
	/*if (StaticMesh->GetLightMapCoordinateIndex() == UVChannelIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid UVChannel: %i. Already used by LightMap"), UVChannelIndex);
		return false;
	}*/

	// Add New UVChannel.
	// UE_LOG(LogTemp, Warning, TEXT("UVChannel: %i. Number of existing UVChannels: %i"), UVChannelIndex, StaticMesh->GetNumUVChannels(LODIndex));
	if (UVChannelIndex == StaticMesh->GetNumUVChannels(LODIndex))
	{
		if (!StaticMesh->InsertUVChannel(LODIndex, UVChannelIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("Unable to Add UVChannel"));
			return false;
		}
	}
	else if (UVChannelIndex > StaticMesh->GetNumUVChannels(LODIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("UVChannel: %i Out of Range. Number of existing UVChannels: %i"), UVChannelIndex, StaticMesh->GetNumUVChannels(LODIndex));
		return false;
	}

	// -----------------------------------------------------------------------------

	TMap<FVertexInstanceID, FVector2D> TexCoords;

	for (const FVertexInstanceID VertexInstanceID : MeshDescription->VertexInstances().GetElementIDs())
	{
		const FVertexID VertexID = MeshDescription->GetVertexInstanceVertex(VertexInstanceID);
		const int32 VertexIndex = VertexID.GetValue();

		float U = (0.5f / (float)Width) + (VertexIndex % Width) / (float)Width;
		float V = (0.5f / (float)Height) + (VertexIndex / Width) / (float)Height;
		
		TexCoords.Add(VertexInstanceID, FVector2D(U, V));
	}

	// Set Full Precision UVs
	SetFullPrecisionUVs(StaticMesh, true);

	if (StaticMesh->SetUVChannel(LODIndex, UVChannelIndex, TexCoords))
	{		
		// Update and Mark to Save.
		StaticMesh->MarkPackageDirty();

		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to Set UVChannel: %i. TexCoords: %i"), UVChannelIndex, TexCoords.Num());
		return false;
	};

	return false;
}

bool UAnimToTextureBPLibrary::FindBestResolution(
	const int32 NumFrames, const int32 NumElements,
	int32& OutHeight, int32& OutWidth, int32& OutRowsPerFrame,
	const int32 MaxHeight, const int32 MaxWidth, bool bEnforcePowerOfTwo)
{
	if (bEnforcePowerOfTwo)
	{
		OutWidth = 2;
		while (OutWidth < NumElements && OutWidth < MaxWidth)
		{
			OutWidth *= 2;
		}
		OutRowsPerFrame = FMath::CeilToInt(NumElements / (float)OutWidth);

		const int32 TargetHeight = NumFrames * OutRowsPerFrame;
		OutHeight = 2;
		while (OutHeight < TargetHeight)
		{
			OutHeight *= 2;
		}
	}
	else
	{
		OutRowsPerFrame = FMath::CeilToInt(NumElements / (float)MaxWidth);
		OutWidth = FMath::CeilToInt(NumElements / (float)OutRowsPerFrame);
		OutHeight = NumFrames * OutRowsPerFrame;
	}

	const bool bValidResolution = OutWidth <= MaxWidth && OutHeight <= MaxHeight;
	return bValidResolution;
};

void UAnimToTextureBPLibrary::SetFullPrecisionUVs(UStaticMesh* StaticMesh, bool bFullPrecision)
{
	int32 NumSourceModels = StaticMesh->GetNumSourceModels();
	for (int32 LodIndex = 0; LodIndex < NumSourceModels; LodIndex++)
	{
		FStaticMeshSourceModel& SourceModel = StaticMesh->GetSourceModel(LodIndex);
		SourceModel.BuildSettings.bUseFullPrecisionUVs = bFullPrecision;
	}
}

void UAnimToTextureBPLibrary::SetStaticMeshBoundsExtensions(
	UStaticMesh* StaticMesh, 
	const FVector& PositiveBoundsExtension, 
	const FVector& NegativeBoundsExtension)
{
	if (IsValid(StaticMesh))
	{
		StaticMesh->SetPositiveBoundsExtension(PositiveBoundsExtension);
		StaticMesh->SetNegativeBoundsExtension(NegativeBoundsExtension);
	}
}
