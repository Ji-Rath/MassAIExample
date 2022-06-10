// Copyright 2019-2021 Rexocrates. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "Misc/Guid.h"
#include "IAnimationEditor.h"
#include "IAnimationEditorModule.h"

#include "ISkeletalMeshEditor.h"
#include "ISkeletalMeshEditorModule.h"


class UDebugSkelMeshComponent;
class IPersonaPreviewScene;

class FVertexAnimToolsetEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:
	TSharedRef<FExtender> GetAnimationEditorToolbarExtender(const TSharedRef<FUICommandList> CommandList, TSharedRef<IAnimationEditor> InAnimationEditor);

	//void AddAnimationEditorToolbarExtender();
	//void RemoveAnimationEditorToolbarExtender();

	void HandleAddSkeletalMeshActionExtenderToToolbar(FToolBarBuilder& ParentToolbarBuilder, UDebugSkelMeshComponent* InMeshComponent);


	void AddSkeletalMeshEditorToolbarExtender();

	void RemoveSkeletalMeshEditorToolbarExtender();

	TSharedRef<FExtender> GetSkeletalMeshEditorToolbarExtender(const TSharedRef<FUICommandList> CommandList, TSharedRef<ISkeletalMeshEditor> InSkeletalMeshEditor);

	void DoBakeProcess(UDebugSkelMeshComponent* PreviewComponent) {};

	FDelegateHandle ModuleLoadedDelegateHandle;
	//FDelegateHandle AnimationEditorExtenderHandle;
	FDelegateHandle SkeletalMeshEditorExtenderHandle;
private:
};
