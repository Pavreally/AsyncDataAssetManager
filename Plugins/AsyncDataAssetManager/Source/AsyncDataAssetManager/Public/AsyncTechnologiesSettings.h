// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AsyncTechnologiesSettings.generated.h"

/**
 * Async Plugin Technologies Settings
 */
UCLASS(Config = Plugins, DefaultConfig, meta = (DisplayName = "Async Technologies - ADAM"))
class ASYNCDATAASSETMANAGER_API UAsyncTechnologiesSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UAsyncTechnologiesSettings();

	UPROPERTY(Config, EditAnywhere, Category = "ADAM Subsystem", meta = (DisplayName = "Enable log", ToolTip = "Enables logging for the ADAM subsystem, which notifies about the asynchronous loading and unloading of Data Assets from memory."))
	bool bEnableLogADAM = false;
};
