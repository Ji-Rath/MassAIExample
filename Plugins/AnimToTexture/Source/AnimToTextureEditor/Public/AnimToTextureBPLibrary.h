// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AnimToTextureDataAsset.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Materials/MaterialLayersFunctions.h"

#include "AnimToTextureBPLibrary.generated.h"

class UStaticMesh;
class USkeletalMesh;

/*
* TODO: 
*	- Right now it is saving data per-vertex instead of per-point.
*     This will require more pixels if the mesh has lots of material slots or uv-shells.
*     FStaticMeshOperations::FindOverlappingCorners(FOverlappingCorners& OutOverlappingCorners, const FMeshDescription& MeshDescription, float ComparisonThreshold)
*/
UCLASS()
class UAnimToTextureBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

#if WITH_EDITOR

	/**
	* Bakes Animation Data into Textures.
	* Position & Normals.
	*/
	UFUNCTION(BlueprintCallable, meta = (Category = "VertexAnimationdTesting"))
	static void AnimationToTexture(UAnimToTextureDataAsset* DataAsset,
		const FTransform RootTransform, const bool bCreateTextures=true, const bool bCreateUVChannel=true);

	/* Utility for converting SkeletalMesh into a StaticMesh */
	UFUNCTION(BlueprintCallable, Category = "VertexAnimationdTesting")
	static UStaticMesh* ConvertSkeletalMeshToStaticMesh(USkeletalMesh* SkeletalMesh, const FString PackageName, const int32 LODIndex = -1);

	/**
	* Updates a material's parameters to match those of an animToTexture data asset
	*/
	UFUNCTION(BlueprintCallable, meta = (Category = "VertexAnimationdTesting"))
	static void UpdateMaterialInstanceFromDataAsset(UAnimToTextureDataAsset* DataAsset, class UMaterialInstanceConstant* MaterialInstance, 
		const FAnimToTextureMaterialParamNames& ParamNames, const EMaterialParameterAssociation MaterialParameterAssociation = EMaterialParameterAssociation::GlobalParameter);

	/**
	 * Replaces material layer function
	 */
	UFUNCTION(BlueprintCallable, meta = (Category = "VertexAnimationdTesting"))
	static void UpdateMaterialLayerFunction(
		class UMaterialInstanceConstant* MaterialInstance, 
		class UMaterialFunctionInterface* OldMaterialFunction, 
		class UMaterialFunctionInterface* NewMaterialFunction);

	UFUNCTION(BlueprintCallable, meta = (Category = "VertexAnimationdTesting"))
	static void SetStaticMeshBoundsExtensions(UStaticMesh* StaticMesh, const FVector& PositiveBoundsExtension, const FVector& NegativeBoundsExtension);

#endif // WITH_EDITOR

private:

	static void NormalizeVertexData(
		const TArray<FVector3f>& Deltas, const TArray<FVector3f>& Normals,
		FVector& MinBBox, FVector& SizeBBox,
		TArray<FVector3f>& NormalizedDeltas, TArray<FVector3f>& NormalizedNormals);

	static void NormalizeBoneData(
		const TArray<FVector3f>& Positions, const TArray<FVector4>& Rotations,
		FVector& MinBBox, FVector& SizeBBox,
		TArray<FVector3f>& NormalizedPositions, TArray<FVector4>& NormalizedRotations);

	/* Returns best resolution for the given data. 
	*  Returns false if data wont fit in the the max range.
	*/
	static bool FindBestResolution(const int32 NumFrames, const int32 NumElements,
								   int32& OutHeight, int32& OutWidth, int32& OutRowsPerFrame,
								   const int32 MaxHeight = 4096, const int32 MaxWidth = 4096, bool bEnforcePowerOfTwo = false);

	/* Sets Static Mesh FullPrecisionUVs Property*/
	static void SetFullPrecisionUVs(UStaticMesh* StaticMesh, bool bFullPrecision);

	/* Creates UV Coord with vertices */
	static bool CreateUVChannel(UStaticMesh* StaticMesh, const int32 LODIndex, const int32 UVChannelIndex,
		const int32 Height, const int32 Width);

};
