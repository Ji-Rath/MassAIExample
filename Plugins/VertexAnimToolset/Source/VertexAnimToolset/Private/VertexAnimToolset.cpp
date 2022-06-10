// Copyright 2019-2021 Rexocrates. All Rights Reserved.

#include "VertexAnimToolset.h"

//--------------------------------------
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

#define LOCTEXT_NAMESPACE "FVertexAnimToolsetModule"

void FVertexAnimToolsetModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FVertexAnimToolsetModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVertexAnimToolsetModule, VertexAnimToolset)