#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"


/**
 * The public interface to this module
 */
class BlueprintSorting : public IModuleInterface
{

public:
    /** IModuleInterface implementation */
    void StartupModule();
    void ShutdownModule();
};

