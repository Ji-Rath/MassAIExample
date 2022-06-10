// Copyright 2019-2021 Rexocrates. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Engine/Texture2D.h"
#include "IDetailsView.h"


#include "Layout/Visibility.h"
#include "Widgets/SWidget.h"

#include "IDetailCustomization.h"

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SWindow.h"

#include "ContentBrowserDelegates.h"

class UTextureRenderTarget2D;

class FSkeletalMeshLODRenderData;
class FSkinWeightVertexBuffer;
struct FActiveMorphTarget;
class UVertexAnimProfile;

// Abstract class holding helper functions to be used in the baking process
class FVertexAnimUtils
{
public:

	static float EncodeFloat(const float& T, const float& Bound);
	static float DeEncodeFloat(const float& T, const float& Bound);
	
	static FVector DeEncodeVec(const FVector4& T, const float& Bound);

	static FVector4 BitEncodeVecId(const FVector T, const float Bound, const int32 Id);
	static FVector4 BitEncodeVecId_HD(const FVector T, const float Bound, const int32 Id);

	static int32 Grid2DIndex(const int32& X, const int32& Y, const int32& Width);
	static int32 Grid2D_X(const int32& Index, const int32& Height);
	static int32 Grid2D_Y(const int32& Index, const int32& Height);


	
	/**
	 * Convert a set of mesh components in their current pose to a static mesh.
	 * @param	InMeshComponents		The mesh components we want to convert
	 * @param	InRootTransform			The transform of the root of the mesh we want to output
	 * @param	InPackageName			The package name to create the static mesh in. If this is empty then a dialog will be displayed to pick the mesh.
	 * @return a new static mesh (specified by the user)
	 */
	static UStaticMesh* ConvertMeshesToStaticMesh(const TArray<UMeshComponent*>& InMeshComponents, const FTransform& InRootTransform = FTransform::Identity, const FString& InPackageName = FString());

	static void VATUVsToStaticMeshLODs(UStaticMesh* StaticMesh, const int32 UVChannel, const TArray <TArray <FVector2D>>& UVs);
	static void VATColorsToStaticMeshLODs(UStaticMesh* StaticMesh, const TArray <TArray <FColor>>& Colors);

};


#define LOCTEXT_NAMESPACE "PickAssetDialog"
// dialog for using the bake anim tool in the editor
class SPickAssetDialog : public SWindow
{
public:
	SLATE_BEGIN_ARGS(SPickAssetDialog)
	{
	}
	SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(FText, DefaultAssetPath)
		SLATE_END_ARGS()

		VERTEXANIMTOOLSETEDITOR_API SPickAssetDialog()
		: UserResponse(EAppReturnType::Cancel)
	{
	}

	VERTEXANIMTOOLSETEDITOR_API void Construct(const FArguments& InArgs);

	/** Displays the dialog in a blocking fashion */
	VERTEXANIMTOOLSETEDITOR_API EAppReturnType::Type ShowModal();

	/** Gets the resulting asset path */
	VERTEXANIMTOOLSETEDITOR_API const FText& GetAssetPath();

	/** Gets the resulting asset name */
	VERTEXANIMTOOLSETEDITOR_API const FText& GetAssetName();

	/** Gets the resulting full asset path (path+'/'+name) */
	VERTEXANIMTOOLSETEDITOR_API FText GetFullAssetPath();

	void OnCheckedOnlyCreateStaticMesh(ECheckBoxState NewCheckedState);

	UVertexAnimProfile* GetSelectedProfile() const;

	bool GetOnlyCreateStaticMesh() const
	{
		return bOnlyCreateStaticMesh;
	}

	bool Filter(const FAssetData& AssetData);

	int32 ValidateProfile() const;

private:
	/** Used to get the currently selected assets */
	FGetCurrentSelectionDelegate GetCurrentSelectionDelegate_Profile;

protected:
	void OnPathChange(const FString& NewPath);
	void OnNameChange(const FText& NewName, ETextCommit::Type CommitInfo);
	FReply OnButtonClick(EAppReturnType::Type ButtonID);

	bool ValidatePackage();

	EAppReturnType::Type UserResponse;

	FText AssetPath;
	FText AssetName;

	bool bOnlyCreateStaticMesh = false;
};

#undef LOCTEXT_NAMESPACE
