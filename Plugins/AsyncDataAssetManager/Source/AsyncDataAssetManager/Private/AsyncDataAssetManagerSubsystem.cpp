// Pavel Gornostaev <https://github.com/Pavreally>

#include "AsyncDataAssetManagerSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/DataAsset.h"
#include "AsyncTechnologiesSettings.h"
#include "GameplayTagsManager.h"

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

#pragma region LOADING_FUNCTIONS
void UAsyncDataAssetManagerSubsystem::LoadADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FGameplayTag Tag, bool RecursiveLoading, TSoftObjectPtr<UPrimaryDataAsset>& ReturnPrimaryDataAsset)
{
	if (PrimaryDataAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Load): No reference is specified in function."));
		
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

	// Add in array ADAM and async load. In this case, a load notification occurs after each file is loaded.
	AddToADAM(PrimaryDataAsset, Tag.GetTagName(), RecursiveLoading);

	// Return the value of a soft link
	ReturnPrimaryDataAsset = PrimaryDataAsset;
}

void UAsyncDataAssetManagerSubsystem::LoadArrayADAM(TArray<TSoftObjectPtr<UPrimaryDataAsset>> PrimaryDataAssets, FGameplayTag Tag, bool NotifyAfterFullLoaded, bool RecursiveLoading, TArray<TSoftObjectPtr<UPrimaryDataAsset>>& ReturnPrimaryDataAssets)
{
	if (PrimaryDataAssets.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Load Array): No reference is specified in function."));
		
		return;
	}

	FName ExtendedTag = Tag.GetTagName();

	// Initiate counter for NotifyAfterFullLoaded
	if (NotifyAfterFullLoaded && !QueueCounterADAM.Contains(ExtendedTag))
	{
		QueueCounterADAM.Add(ExtendedTag, 0);
	}

	// Invoking asynchronous loading of each data asset.
	for (TSoftObjectPtr<UPrimaryDataAsset>& DataAsset : PrimaryDataAssets)
	{
		// Stop execution if there is a duplicate in memory
		if (!NotifyAfterFullLoaded && GetIndexDataADAM(DataAsset) >= 0)
		{
			if (EnableLog)
			{
				UE_LOG(LogTemp, Warning, TEXT("ADAM (Load Array): You are trying to load the same Data Asset \"%s\" twice."), *DataAsset.GetAssetName());
			}

			continue;
		}
		
		// Add in array ADAM and async load
		if (!NotifyAfterFullLoaded)
		{
			AddToADAM(DataAsset, ExtendedTag, RecursiveLoading);
		}
		else
		{
			AddAllToADAM(DataAsset, ExtendedTag, RecursiveLoading);
		}
	}

	// Return an array of soft links
	ReturnPrimaryDataAssets = PrimaryDataAssets;
}

void UAsyncDataAssetManagerSubsystem::AddToADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, bool RecursiveLoading)
{
	// Add Queue
	FString DataAssetName = PrimaryDataAsset.GetAssetName();

	if (QueueADAM.Contains(DataAssetName)) 
		return;

	QueueADAM.Add(DataAssetName);

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	// Create a delegate
	FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(
	this,
	&UAsyncDataAssetManagerSubsystem::OnLoaded,
	PrimaryDataAsset,
	FGameplayTag::RequestGameplayTag(Tag, false),
	RecursiveLoading);

	// Determine whether the descriptor will be declared and stored
	TSharedPtr<FStreamableHandle> DataAssetHandle = StreamableManager.RequestAsyncLoad(PrimaryDataAsset.ToSoftObjectPath(),	Delegate);
	
	// Adding new data
	AddDataToArrayADAM(PrimaryDataAsset, DataAssetHandle, Tag);
}

void UAsyncDataAssetManagerSubsystem::AddAllToADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, bool RecursiveLoading)
{
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	// Create a delegate
	FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(
	this,
	&UAsyncDataAssetManagerSubsystem::OnAllLoaded,
	PrimaryDataAsset,
	FGameplayTag::RequestGameplayTag(Tag, false),
	RecursiveLoading);

	// Determine whether the descriptor will be declared and stored
	TSharedPtr<FStreamableHandle> DataAssetHandle = StreamableManager.RequestAsyncLoad(PrimaryDataAsset.ToSoftObjectPath(),	Delegate);
	
	// Adding new data
	AddDataToArrayADAM(PrimaryDataAsset, DataAssetHandle, Tag);

	// Increment the counter of data
	if (QueueCounterADAM.Contains(Tag)) QueueCounterADAM[Tag]++;
}

void UAsyncDataAssetManagerSubsystem::AddDataToArrayADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, TSharedPtr<FStreamableHandle> DataAssetHandle, FName Tag)
{
	FMemoryADAM NewDataAsset;
	NewDataAsset.SoftReference = PrimaryDataAsset;
	NewDataAsset.MemoryReference = DataAssetHandle;
	NewDataAsset.Tag = Tag;

	DataADAM.Add(NewDataAsset);
}

void UAsyncDataAssetManagerSubsystem::FastLoadADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, TSoftObjectPtr<UPrimaryDataAsset>& ReturnPrimaryDataAsset)
{
	if (PrimaryDataAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Fast Load): No reference is specified in function."));

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
	// Create an empty argument to populate the delegate
	FName Tag;
	FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(
			this,
			&UAsyncDataAssetManagerSubsystem::OnLoaded,
			PrimaryDataAsset,
			FGameplayTag::RequestGameplayTag(Tag, false),
			false);

	// Determine whether the descriptor will be declared and stored
	TSharedPtr<FStreamableHandle> DataAssetHandle = StreamableManager.RequestAsyncLoad(PrimaryDataAsset.ToSoftObjectPath(),	Delegate);
	
	// Return soft reference of Data Asset
	ReturnPrimaryDataAsset = PrimaryDataAsset;
}

#pragma endregion LOADING_FUNCTIONS

#pragma region CALL_DELEGATE
void UAsyncDataAssetManagerSubsystem::OnLoaded(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FGameplayTag Tag, bool RecursiveLoading)
{
	UPrimaryDataAsset* LoadedObject =	PrimaryDataAsset.Get();

	if (!LoadedObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (On Loaded): Received a null value."));

		return;
	}

	// Inform the FOnLoadedADAM subsystem delegate that the loading is complete
	OnLoadedADAM.Broadcast(LoadedObject, PrimaryDataAsset, Tag, RecursiveLoading);

	if (RecursiveLoading && FindNestedAssets(LoadedObject).Num() != 0)
	{
		RecursiveLoad(PrimaryDataAsset, Tag.GetTagName(), false);
	}

	if (EnableLog)
	{
		UE_LOG(LogTemp, Display, TEXT("ADAM (On Loaded): Data Asset \"%s\" is loaded."), *PrimaryDataAsset.GetAssetName());
	}

	// Clear Queue
	QueueADAM.Remove(PrimaryDataAsset.GetAssetName());
}

void UAsyncDataAssetManagerSubsystem::OnAllLoaded(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FGameplayTag Tag, bool RecursiveLoading)
{
	UPrimaryDataAsset* LoadedObject =	PrimaryDataAsset.Get();

	if (!LoadedObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (On Loaded): Received a null value."));

		return;
	}

	FName ExtendedTag = Tag.GetTagName();

	if (RecursiveLoading && FindNestedAssets(LoadedObject).Num() != 0)
	{
		RecursiveLoad(PrimaryDataAsset, ExtendedTag, true);
	}

	if (EnableLog)
	{
		UE_LOG(LogTemp, Display, TEXT("ADAM (On All Loaded): Data Asset \"%s\" is loaded."), *PrimaryDataAsset.GetAssetName());
	}

	if (QueueCounterADAM.Contains(ExtendedTag))
	{
		QueueCounterADAM[ExtendedTag]--;

		// Inform the FOnAllLoadedADAM subsystem delegate that the loading is complete
		if (QueueCounterADAM[ExtendedTag] == 0)
		{
			OnAllLoadedADAM.Broadcast(Tag);

			if (EnableLog)
			{
				UE_LOG(LogTemp, Display, TEXT("ADAM (On All Loaded): All Data Assets has been loaded."), *PrimaryDataAsset.GetAssetName());
			}

			QueueCounterADAM.Remove(ExtendedTag);

			return;
		}
	}
}

#pragma endregion CALL_DELEGATE

#pragma region UNLOADING_FUNCTIONS
void UAsyncDataAssetManagerSubsystem::UnloadADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, bool ForcedUnload)
{
	if (PrimaryDataAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Unload): No reference to Data Asset \"%s \" is specified!"), *PrimaryDataAsset.GetAssetName());
		
		return;
	}

	if (DataADAM.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Unload): Nothing to delete. The \"DataADAM\" array is empty."));

		return;
	}

	// Deletion of target data from ADAM if deletion by tag was not triggered.
	int32 TargetIndex = GetIndexDataADAM(PrimaryDataAsset);

	if (TargetIndex == -1) 
		return;

	if (EnableLog)
	{
		UE_LOG(LogTemp, Display, TEXT("ADAM (Unload): Unload data asset \"%s\" (index: %d)"), *PrimaryDataAsset.GetAssetName(), TargetIndex);
	}

	RemoveFromADAM(TargetIndex, ForcedUnload);
}

void UAsyncDataAssetManagerSubsystem::UnloadAllADAM(bool ForcedUnload)
{
	if (DataADAM.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Unload All ADAM): Nothing to delete. The \"DataADAM\" array is empty."));

		return;
	}

	// Remove all from ADAM
	for (int32 i = DataADAM.Num() - 1; i >= 0; i--)
	{
		if (EnableLog)
		{
			UE_LOG(LogTemp, Display, TEXT("ADAM (Unload All ADAM): Unload data asset (index: %d)"), i);
		}

		RemoveFromADAM(i, ForcedUnload);
	}
}

void UAsyncDataAssetManagerSubsystem::UnloadAllTagsADAM(FGameplayTag Tag, bool ForcedUnload)
{
	FName ExtendedTag = Tag.GetTagName();

	// Remove of all data with a similar target tag
	if (ExtendedTag != "None")
	{
		for (int32 i = DataADAM.Num() - 1; i >= 0; i--)
		{
			if (DataADAM[i].Tag == ExtendedTag)
			{
				if (EnableLog)
				{
					UE_LOG(LogTemp, Display, TEXT("ADAM (Unload All Tags ADAM): Unload data asset \"%s\" (index: %d)"), *DataADAM[i].SoftReference.GetAssetName(), i);
				}

				RemoveFromADAM(i, ForcedUnload);
			}
		}
	}
}

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
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Remove From ADAM): Attempting to unload a Data Asset \"%s\" (index: %d) that does not exist in memory."), *DataADAM[DataAssetIndex].SoftReference.GetAssetName(), DataAssetIndex);
		
		return;
	}
}

#pragma endregion UNLOADING_FUNCTIONS

#pragma region GETTING_DATA_FUNCTIONS
TArray<FMirrorADAM> UAsyncDataAssetManagerSubsystem::GetDataADAM()
{
	int32 Length = DataADAM.Num();
	TArray<FMirrorADAM> MirrorDataADAM;

	if (Length == 0) 
		return MirrorDataADAM;

	for (int32 i = 0; i < Length; i++)
	{
		FMirrorADAM MirrorDataAsset;
		MirrorDataAsset.PrimaryDataAssetName = DataADAM[i].SoftReference.GetAssetName();
		MirrorDataAsset.SoftReference = DataADAM[i].SoftReference;
		MirrorDataAsset.Tag = DataADAM[i].Tag;
		MirrorDataADAM.Add(MirrorDataAsset);
	}

	return MirrorDataADAM;
}

TMap<FName, int32> UAsyncDataAssetManagerSubsystem::GetCollectionByTagADAM()
{
	TMap<FName, int32> TagCollection;

	if (DataADAM.Num() == 0) 
		return TagCollection;

	for (const FMemoryADAM& Data : DataADAM)
	{
		if (TagCollection.Contains(Data.Tag))
		{
			TagCollection[Data.Tag]++;
		}
		else
		{
			TagCollection.Add(Data.Tag, 1);
		}
	}

	return TagCollection;
}

UObject* UAsyncDataAssetManagerSubsystem::GetObjectDataADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, bool& IsValid )
{
	if (PrimaryDataAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Get Object): No reference is specified in function."));

		return nullptr;
	}

	int32 ObjectIndex = GetIndexDataADAM(PrimaryDataAsset);

	if (ObjectIndex == -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Get Object): The requested data asset \"%s\" is not in the memory of the ADAM subsystem."), *PrimaryDataAsset.GetAssetName());
		
		return nullptr;
	}

	UObject* DataAsset = DataADAM[ObjectIndex].MemoryReference->GetLoadedAsset();

	// Return bool value. Checking of Data Asset
	IsValid = DataAsset != nullptr;

	return DataAsset;
}

int32 UAsyncDataAssetManagerSubsystem::GetIndexDataADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset)
{
	for (int32 i = 0; i < DataADAM.Num(); i++)
	{
		if (DataADAM[i].SoftReference == PrimaryDataAsset) 
			return i;
	}

	return -1;
}

TArray<TSoftObjectPtr<UPrimaryDataAsset>> UAsyncDataAssetManagerSubsystem::FindNestedAssets(UPrimaryDataAsset* DataAsset)
{
	TArray<TSoftObjectPtr<UPrimaryDataAsset>> NestedAssets;

	if (!DataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Recursive Load): Received a null value."));

		return NestedAssets;
	}

	// Storage of unique asset names for duplicate control.
	TArray<FString> UniqueAssetNames;

	// Use reflection to search for nested Data Assets
	for (TFieldIterator<FProperty> PropIterator(DataAsset->GetClass()); PropIterator; ++PropIterator)
	{
		const FProperty* Prop = *PropIterator;

		// Look for properties of type TSoftObjectPtr<UPrimaryDataAsset>
		if (const FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Prop))
		{
			if (SoftObjectProperty->PropertyClass->IsChildOf(UPrimaryDataAsset::StaticClass()))
			{
				TSoftObjectPtr<UPrimaryDataAsset> ChildAsset = *SoftObjectProperty->ContainerPtrToValuePtr<TSoftObjectPtr<UPrimaryDataAsset>>(DataAsset);
				FString ChildAssetName = ChildAsset.GetAssetName();
				// Filtering and adding data
				if (!ChildAssetName.IsEmpty() && !UniqueAssetNames.Contains(ChildAssetName))
				{
					UniqueAssetNames.AddUnique(ChildAssetName);
					NestedAssets.Add(ChildAsset);
				}
			}
		}

		// If the array of soft references
		if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(Prop))
		{
			if (const FSoftObjectProperty* InnerSoftObjectProperty = CastField<FSoftObjectProperty>(ArrayProp->Inner))
			{
				if (InnerSoftObjectProperty->PropertyClass->IsChildOf(UPrimaryDataAsset::StaticClass()))
				{
					FScriptArrayHelper ArrayHelper(ArrayProp, ArrayProp->ContainerPtrToValuePtr<void>(DataAsset));

					for (int32 i = 0; i < ArrayHelper.Num(); ++i)
					{
						TSoftObjectPtr<UPrimaryDataAsset> ChildAsset = *reinterpret_cast<TSoftObjectPtr<UPrimaryDataAsset>*>(ArrayHelper.GetRawPtr(i));
						FString ChildAssetName = ChildAsset.GetAssetName();
						// Filtering and adding data
						if (!ChildAssetName.IsEmpty() && !UniqueAssetNames.Contains(ChildAssetName))
						{
							UniqueAssetNames.AddUnique(ChildAssetName);
							NestedAssets.Add(ChildAsset);
						}
					}
				}
			}
		}
	}

	if (EnableLog && NestedAssets.Num() == 0)
	{
		UE_LOG(LogTemp, Display, TEXT("ADAM (Recursive data): Iteration is complete. All nested data is loaded!"));
	}

	return NestedAssets;
}

#pragma endregion GETTING_DATA_FUNCTIONS

void UAsyncDataAssetManagerSubsystem::RecursiveLoad(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, bool NotifyAfterFullLoaded)
{
	if (PrimaryDataAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Recursive Load): No reference is specified in function."));
		
		return;
	}

	UPrimaryDataAsset* Asset = PrimaryDataAsset.Get();

	if (!Asset)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Recursive Load): Received a null value."));
		
		return;
	}

	// Checking nested files
	TArray<TSoftObjectPtr<UPrimaryDataAsset>> NestedAssets = FindNestedAssets(Asset);

	if (NestedAssets.Num() == 0)
	{
		if (EnableLog)
		{
			UE_LOG(LogTemp, Warning, TEXT("ADAM (Recursive Load): No nested files were found in file \"%s\"."), *PrimaryDataAsset.GetAssetName());
		}

		return;
	}

	for (TSoftObjectPtr<UPrimaryDataAsset>& NestedAsset : NestedAssets)
	{
		// Calling asynchronous loading
		if (!NotifyAfterFullLoaded)
		{
			// Stop execution if there is a duplicate in memory
			if (GetIndexDataADAM(NestedAsset) >= 0)
			{
				if (EnableLog)
				{
					UE_LOG(LogTemp, Warning, TEXT("ADAM (Recursive Load): You are trying to load the same Data Asset \"%s\" twice."), *NestedAsset.GetAssetName());
				}

				continue;
			}

			AddToADAM(NestedAsset, Tag, true);
		}
		else
		{
			AddAllToADAM(NestedAsset, Tag, true);
		}
	}
}
