/**
* Copyright (c) 2018 Cronofear Softworks, Inc. All Rights Reserved.
*
* Developed by Kevin Yabar Garces
*/

#include "CSWAutoSaveAndLoadSystem.h"

#define LOCTEXT_NAMESPACE "FCSWAutoSaveAndLoadSystemModule"

void FCSWAutoSaveAndLoadSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FCSWAutoSaveAndLoadSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
///PLUGIN
IMPLEMENT_MODULE(FCSWAutoSaveAndLoadSystemModule, CSWAutoSaveAndLoadSystem)
///MODULE
//IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, CSWAutoSaveAndLoadSystem, "CSWAutoSaveAndLoadSystem");