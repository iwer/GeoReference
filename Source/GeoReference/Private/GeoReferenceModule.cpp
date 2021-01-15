// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "UnrealGDAL.h"

#define LOCTEXT_NAMESPACE "FGeoReferenceModule"

class FGeoReferenceModule : public IModuleInterface
{
public:

    /** IModuleInterface implementation */
    virtual void StartupModule() override
    {
        // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
        // Init GDAL module
        FUnrealGDALModule* UnrealGDAL = FModuleManager::Get().LoadModulePtr<FUnrealGDALModule>("UnrealGDAL");
        UnrealGDAL->InitGDAL();
    }
    virtual void ShutdownModule() override
    {
        // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
        // we call this function before unloading the module.
    }
};



#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGeoReferenceModule, GeoReference)
