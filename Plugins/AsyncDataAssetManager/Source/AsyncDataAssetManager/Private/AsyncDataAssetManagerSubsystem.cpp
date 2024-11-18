// Pavel Gornostaev <https://github.com/Pavreally>

#include "AsyncDataAssetManagerSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/DataAsset.h"
#include "AsyncTechnologiesSettings.h"

// Initialize subsystem
void UAsyncDataAssetManagerSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
	Super::Initialize(Collection);

	// Get the list of settings for the ADAM subsystem 
	const UAsyncTechnologiesSettings* SettingsADAM = GetDefault<UAsyncTechnologiesSettings>();
	EnableLog = SettingsADAM->bEnableLogADAM;

	// Allows you to use Blueprint Code
	void ReceivePostSubsystemInit();
}

// Freeing memory during deinitialization
void UAsyncDataAssetManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	DataADAM.Empty();

	OnLoadedADAM.Clear();
}

// BlueprintCallable Function. Asynchronous loading of Primary Data Asset
void UAsyncDataAssetManagerSubsystem::LoadADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, TSoftObjectPtr<UPrimaryDataAsset> &ReturnPrimaryDataAsset)
{
	if (PrimaryDataAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Load): No reference is specified in the \"LoadADAM\" function."));
		return;
	}

	// Stop execution if there is a duplicate in memory
	if (GetIndexDataADAM(PrimaryDataAsset) >= 0)
	{
		if (EnableLog)
		{
			UE_LOG(LogTemp, Warning, TEXT("ADAM (Load): You are trying to load the same Data Asset \"%s\" twice."), *PrimaryDataAsset.GetAssetName());
		}
		return;
	}

	// Add in array ADAM and async load
	AddToADAM(PrimaryDataAsset);

	// Return the value of a soft link
	ReturnPrimaryDataAsset = PrimaryDataAsset;
}

// BlueprintCallable Function. Asynchronous loading of an array of Primary Data Asset
void UAsyncDataAssetManagerSubsystem::LoadArrayADAM(TArray<TSoftObjectPtr<UPrimaryDataAsset>> PrimaryDataAssets, TArray<TSoftObjectPtr<UPrimaryDataAsset>> &ReturnPrimaryDataAssets)
{
	if (PrimaryDataAssets.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Load Array): No reference is specified in the \"LoadADAM\" function."));
		return;
	}

	for (TSoftObjectPtr<UPrimaryDataAsset> DataAsset : PrimaryDataAssets)
	{
		// Stop execution if there is a duplicate in memory
		if (GetIndexDataADAM(DataAsset) >= 0)
		{
			if (EnableLog)
			{
				UE_LOG(LogTemp, Warning, TEXT("ADAM (Load Array): You are trying to load the same Data Asset \"%s\" twice."), *DataAsset.GetAssetName());
			}
			continue;
		}

		// Add in array ADAM and async load
		AddToADAM(DataAsset);
	}

	// Return an array of soft links
	ReturnPrimaryDataAssets = PrimaryDataAssets;
}

// Add Data Asset to the ADAM array and load asynchronously
void UAsyncDataAssetManagerSubsystem::AddToADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset)
{
	// Create a delegate
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(
			this,
			&UAsyncDataAssetManagerSubsystem::OnLoaded,
			PrimaryDataAsset);

	// Add Queue
	FString DataAssetName = PrimaryDataAsset.GetAssetName();
	
	if (QueueADAM.Find(DataAssetName) != INDEX_NONE) 
	{
		return;
	}

	QueueADAM.Add(DataAssetName);

	// Determine whether the descriptor will be declared and stored
	TSharedPtr<FStreamableHandle> DataAssetHandle = StreamableManager.RequestAsyncLoad(PrimaryDataAsset.ToSoftObjectPath(),	Delegate);
	
	FMemoryADAM NewDataAsset;
	NewDataAsset.SoftReference = PrimaryDataAsset;
	NewDataAsset.MemoryReference = DataAssetHandle;
	DataADAM.Add(NewDataAsset);
}

// Delegate notification after loading Data Asset into ADAM subsystem
void UAsyncDataAssetManagerSubsystem::OnLoaded(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset)
{
	UObject* LoadedObject =	PrimaryDataAsset.Get();

	// Inform the FOnLoadedADAM subsystem delegate that the loading is complete
	OnLoadedADAM.Broadcast(LoadedObject, PrimaryDataAsset);

	if (EnableLog)
	{
		UE_LOG(LogTemp, Display, TEXT("ADAM: Data Asset \"%s\" is loaded."), *PrimaryDataAsset.GetAssetName());
	}

	// Clear Queue
	QueueADAM.Remove(PrimaryDataAsset.GetAssetName());
}

// Loading a Data Asset without storing it in memory
void UAsyncDataAssetManagerSubsystem::FastLoadADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, TSoftObjectPtr<UPrimaryDataAsset> &ReturnPrimaryDataAsset)
{
	if (PrimaryDataAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Fast Load): No reference is specified in the \"LoadADAM\" function."));
		return;
	}

	// Stop execution if there is a duplicate in memory
	if (GetIndexDataADAM(PrimaryDataAsset) >= 0)
	{
		if (EnableLog)
		{
			UE_LOG(LogTemp, Warning, TEXT("ADAM (Fast Load): You are trying to load the same Data Asset \"%s\" twice."), *PrimaryDataAsset.GetAssetName());
		}
		return;
	}

	// Create a delegate
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(
			this,
			&UAsyncDataAssetManagerSubsystem::OnLoaded,
			PrimaryDataAsset);

	// Determine whether the descriptor will be declared and stored
	TSharedPtr<FStreamableHandle> DataAssetHandle = StreamableManager.RequestAsyncLoad(PrimaryDataAsset.ToSoftObjectPath(),	Delegate);
	
	// Return soft reference of Data Asset
	ReturnPrimaryDataAsset = PrimaryDataAsset;
}

// BlueprintCallable Function. Unload an item from the ADAM collection
void UAsyncDataAssetManagerSubsystem::UnloadADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, bool ForcedUnload)
{
	if (PrimaryDataAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Unload): No reference to Data Asset \"%s \" is specified!"), *PrimaryDataAsset.GetAssetName());
		return;
	}

	if (DataADAM.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Nothing to delete. The \"DataADAM\" array is empty."));
		return;
	}

	int32 TargetIndex = GetIndexDataADAM(PrimaryDataAsset);

	RemoveFromADAM(TargetIndex, ForcedUnload);

	if (EnableLog)
	{
		UE_LOG(LogTemp, Display, TEXT("ADAM: Unload data asset \"%s\" (index: %d)"), *PrimaryDataAsset.GetAssetName(), TargetIndex);
	}
}

// BlueprintCallable Function. Unload all items from the ADAM collection
void UAsyncDataAssetManagerSubsystem::UnloadAllADAM(bool ForcedUnload)
{
	if (DataADAM.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Nothing to delete. The \"DataADAM\" array is empty."));
		return;
	}

	for (int32 i = DataADAM.Num() - 1; i >= 0; i--)
	{
		RemoveFromADAM(i, ForcedUnload);

		if (EnableLog)
		{
			UE_LOG(LogTemp, Display, TEXT("ADAM: Unload data asset (index: %d)"), i);
		}
	}
}

// Remove Data Asset from the ADAM array and asynchronously unload it
void UAsyncDataAssetManagerSubsystem::RemoveFromADAM(int32 DataAssetIndex, bool ForcedUnload)
{
	// Stop execution if there is a duplicate in memory
	if (DataADAM.IsValidIndex(DataAssetIndex))
	{
		// Release Handle and tell the system that the data in memory is no longer needed
		DataADAM[DataAssetIndex].MemoryReference->ReleaseHandle();
		if (ForcedUnload)
		{
			DataADAM[DataAssetIndex].MemoryReference.Reset();
		}

		// Remove the target element from the main array
		DataADAM.RemoveAt(DataAssetIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Unload): Attempting to unload a Data Asset \"%s\" (index: %d) that does not exist in memory."), *DataADAM[DataAssetIndex].SoftReference.GetAssetName(), DataAssetIndex);
		return;
	}
}

// BlueprintCallable Function. Return the length of the main array of the subsystem - DataADAM 
void UAsyncDataAssetManagerSubsystem::GetDataADAM(TArray<FMirrorADAM> &MirrorDataADAM, int32 &Length)
{
	Length = DataADAM.Num();

	if (Length >= 0)
	{
		for (int32 i = 0; i < Length; i++)
		{
			FMirrorADAM MirrorDataAsset;
			MirrorDataAsset.PrimaryDataAssetName = DataADAM[i].SoftReference.GetAssetName();
			MirrorDataAsset.SoftReference = DataADAM[i].SoftReference;
			MirrorDataADAM.Add(MirrorDataAsset);
		}
	}
}

// BlueprintCallable Function. Returns a pointer to the Data Asset (ADAM) object stored in memory.
UObject* UAsyncDataAssetManagerSubsystem::GetObjectDataADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, bool &IsValid )
{
	if (PrimaryDataAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Get Object): No reference is specified in the \"GetObjectDataADAM\" function."));
		return nullptr;
	}

	int32 ObjectIndex = GetIndexDataADAM(PrimaryDataAsset);

	if (ObjectIndex == -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("The requested data asset \"%s\" is not in the memory of the ADAM subsystem."), *PrimaryDataAsset.GetAssetName());
		return nullptr;
	}

	UObject* DataAsset = DataADAM[ObjectIndex].MemoryReference->GetLoadedAsset();

	// Return bool value. Checking of Data Asset
	IsValid = DataAsset != nullptr;

	return DataAsset;
}

// Returns the index from the array of the Data Assets (ADAM) collection. If nothing is found returns -1.
int32 UAsyncDataAssetManagerSubsystem::GetIndexDataADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset)
{
	for (int32 i = 0; i < DataADAM.Num(); i++)
	{
		if (DataADAM[i].SoftReference == PrimaryDataAsset)
		{
			return i;
		}
	}

	return -1;
}
