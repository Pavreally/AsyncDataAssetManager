// Pavel Gornostaev <https://github.com/Pavreally>

#include "AsyncDataAssetManagerSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/DataAsset.h"
#include "AsyncTechnologiesSettings.h"

#pragma region SUBSYSTEM
// Initialize subsystem
void UAsyncDataAssetManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get the list of settings for the ADAM subsystem 
	const UAsyncTechnologiesSettings* SettingsADAM = GetDefault<UAsyncTechnologiesSettings>();
	EnableLog = SettingsADAM->bEnableLogADAM;
}

// Freeing memory during deinitialization
void UAsyncDataAssetManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (!DataADAM.IsEmpty())
	{
		// Clearing saved TSharedPtr<FStreamableHandle>
		UnloadAllADAM(true);
	}

	OnLoadedADAM.Clear();
	OnAllLoadedADAM.Clear();
}

#pragma endregion SUBSYSTEM

#pragma region CALL_DELEGATE
void UAsyncDataAssetManagerSubsystem::OnLoaded(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, int32 RecursiveDepthLoading)
{
	UPrimaryDataAsset* LoadedObject =	PrimaryDataAsset.Get();

	if (!LoadedObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (On Loaded): Received a null value."));

		return;
	}

	// Inform the FOnLoadedADAM subsystem delegate that the loading is complete
	OnLoadedADAM.Broadcast(LoadedObject, PrimaryDataAsset, Tag, RecursiveDepthLoading);

	if (RecursiveDepthLoading != 0 && FindNestedAssets(LoadedObject).Num() != 0)
	{
		RecursiveLoad(PrimaryDataAsset, Tag, false, RecursiveDepthLoading);
	}

	if (EnableLog)
	{
		UE_LOG(LogTemp, Display, TEXT("ADAM (On Loaded): Data Asset \"%s\" is loaded."), *PrimaryDataAsset.GetAssetName());
	}

	// Clear Queue
	QueueADAM.Remove(PrimaryDataAsset.GetAssetName());
}

void UAsyncDataAssetManagerSubsystem::OnAllLoaded(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, int32 RecursiveDepthLoading)
{
	UPrimaryDataAsset* LoadedObject =	PrimaryDataAsset.Get();

	if (!LoadedObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (On Loaded): Received a null value."));

		return;
	}

	if (RecursiveDepthLoading != 0 && FindNestedAssets(LoadedObject).Num() != 0)
	{
		RecursiveLoad(PrimaryDataAsset, Tag, true, RecursiveDepthLoading);
	}

	if (EnableLog)
	{
		UE_LOG(LogTemp, Display, TEXT("ADAM (On All Loaded): Data Asset \"%s\" is loaded."), *PrimaryDataAsset.GetAssetName());
	}

	if (QueueCounterADAM.Contains(Tag))
	{
		QueueCounterADAM[Tag]--;

		// Inform the FOnAllLoadedADAM subsystem delegate that the loading is complete
		if (QueueCounterADAM[Tag] == 0)
		{
			OnAllLoadedADAM.Broadcast(Tag);

			if (EnableLog)
			{
				UE_LOG(LogTemp, Display, TEXT("ADAM (On All Loaded): All Data Assets has been loaded."), *PrimaryDataAsset.GetAssetName());
			}

			QueueCounterADAM.Remove(Tag);
		}
	}
}

#pragma endregion CALL_DELEGATE
