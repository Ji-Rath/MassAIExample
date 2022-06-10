// Copyright 2019-2021 Rexocrates. All Rights Reserved.

#include "VertexAnimToolsetEditor.h"

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

#include "VertexAnimUtils.h"
#include "VertexAnimProfile.h"


#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

//#include "Engine.h"

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

#include "Animation/AnimSequence.h"

//#include "Runtime/Engine/Classes/Kismet/KismetRenderingLibrary.h"

#include "Kismet/KismetRenderingLibrary.h"

#include "PackageTools.h"

#include "VATEditorUtils.h"

#define LOCTEXT_NAMESPACE "FVertexAnimToolsetEditorModule"

//#define YESSCENETICK

void FVertexAnimToolsetEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	// hook up level editor extension for skeletal mesh conversion
	
	//FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("VertexAnimToolset"))->GetBaseDir(), TEXT("Shaders"));
	//AddShaderSourceDirectoryMapping(TEXT("/Plugin/VertexAnimToolset"), PluginShaderDir);
	
	
	ModuleLoadedDelegateHandle = FModuleManager::Get().OnModulesChanged().AddLambda([this](FName InModuleName, EModuleChangeReason InChangeReason)
	{
		if (InChangeReason == EModuleChangeReason::ModuleLoaded)
		{
			if (InModuleName == "SkeletalMeshEditor")
			{
				//check(false); TRIGGERED
				AddSkeletalMeshEditorToolbarExtender();
			}
		}
	});
}

void FVertexAnimToolsetEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	//RemoveAnimationEditorToolbarExtender();

	// This is not causing the stuck on exiting Unreal??
	RemoveSkeletalMeshEditorToolbarExtender();
	FModuleManager::Get().OnModulesChanged().Remove(ModuleLoadedDelegateHandle);
}

TSharedRef<FExtender> FVertexAnimToolsetEditorModule::GetAnimationEditorToolbarExtender(const TSharedRef<FUICommandList> CommandList, TSharedRef<IAnimationEditor> InAnimationEditor)
{
	TSharedRef<FExtender> Extender = MakeShareable(new FExtender);

	UDebugSkelMeshComponent* MeshComponent = InAnimationEditor->GetPersonaToolkit()->GetPreviewMeshComponent();
	TSharedRef<IPersonaPreviewScene> Scene = InAnimationEditor->GetPersonaToolkit()->GetPreviewScene();
	
	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		CommandList,
		FToolBarExtensionDelegate::CreateRaw(this, &FVertexAnimToolsetEditorModule::HandleAddSkeletalMeshActionExtenderToToolbar, MeshComponent)
	);

	return Extender;
	
}

void FVertexAnimToolsetEditorModule::HandleAddSkeletalMeshActionExtenderToToolbar(FToolBarBuilder & ParentToolbarBuilder, UDebugSkelMeshComponent * InMeshComponent)
{
	
	ParentToolbarBuilder.AddToolBarButton(
		FUIAction(FExecuteAction::CreateLambda([this, InMeshComponent]()
	{
		FVATEditorUtils::DoBakeProcess(InMeshComponent);
	})),
		NAME_None,
		LOCTEXT("BakeAnim", "Bake Anim"),
		LOCTEXT("BakeAnimToTextureTooltip", "Bake animation frames to textures"),
		FSlateIcon("EditorStyle", "Persona.TogglePreviewAsset", "Persona.TogglePreviewAsset.Small")
		);
}

void FVertexAnimToolsetEditorModule::AddSkeletalMeshEditorToolbarExtender()
{
	ISkeletalMeshEditorModule& SkeletalMeshEditorModule = FModuleManager::Get().LoadModuleChecked<ISkeletalMeshEditorModule>("SkeletalMeshEditor");
	auto& ToolbarExtenders = SkeletalMeshEditorModule.GetAllSkeletalMeshEditorToolbarExtenders();

	ToolbarExtenders.Add(
		ISkeletalMeshEditorModule::FSkeletalMeshEditorToolbarExtender::CreateRaw(this, &FVertexAnimToolsetEditorModule::GetSkeletalMeshEditorToolbarExtender));
	SkeletalMeshEditorExtenderHandle = ToolbarExtenders.Last().GetHandle();
}

void FVertexAnimToolsetEditorModule::RemoveSkeletalMeshEditorToolbarExtender()
{
	ISkeletalMeshEditorModule* SkeletalMeshEditorModule = FModuleManager::Get().GetModulePtr<ISkeletalMeshEditorModule>("SkeletalMeshEditor");
	if (SkeletalMeshEditorModule)
	{
		typedef ISkeletalMeshEditorModule::FSkeletalMeshEditorToolbarExtender DelegateType;
		SkeletalMeshEditorModule->GetAllSkeletalMeshEditorToolbarExtenders().RemoveAll([=](const DelegateType& In) { return In.GetHandle() == SkeletalMeshEditorExtenderHandle; });
	}
}

TSharedRef<FExtender> FVertexAnimToolsetEditorModule::GetSkeletalMeshEditorToolbarExtender(const TSharedRef<FUICommandList> CommandList, TSharedRef<ISkeletalMeshEditor> InSkeletalMeshEditor)
{
	TSharedRef<FExtender> Extender = MakeShareable(new FExtender);

	//UMeshComponent* MeshComponent = InSkeletalMeshEditor->GetPersonaToolkit()->GetPreviewMeshComponent();
	UDebugSkelMeshComponent* MeshComponent = InSkeletalMeshEditor->GetPersonaToolkit()->GetPreviewMeshComponent();
	
	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		CommandList,
		FToolBarExtensionDelegate::CreateRaw(this, &FVertexAnimToolsetEditorModule::HandleAddSkeletalMeshActionExtenderToToolbar, MeshComponent)
	);

	return Extender;
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVertexAnimToolsetEditorModule, VertexAnimToolsetEditor)