// Pavel Gornostaev <https://github.com/Pavreally>

#include "AsyncDataAssetManagerSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/DataAsset.h"
#include "AsyncTechnologiesSettings.h"

void UAsyncDataAssetManagerSubsystem::LoadADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FTagADAM Tag, bool RecursiveLoading, TSoftObjectPtr<UPrimaryDataAsset>& ReturnPrimaryDataAsset)
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
	AddToADAM(PrimaryDataAsset, GetTagNameFromStruct(Tag), RecursiveLoading);

	// Return the value of a soft link
	ReturnPrimaryDataAsset = PrimaryDataAsset;
}

void UAsyncDataAssetManagerSubsystem::LoadArrayADAM(TArray<TSoftObjectPtr<UPrimaryDataAsset>> PrimaryDataAssets, FTagADAM Tag, bool NotifyAfterFullLoaded, bool RecursiveLoading, TArray<TSoftObjectPtr<UPrimaryDataAsset>>& ReturnPrimaryDataAssets)
{
	if (PrimaryDataAssets.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Load Array): No reference is specified in function."));
		
		return;
	}

	// Get and check the type of tag used in a function
	FName TagName = GetTagNameFromStruct(Tag);

	// Initiate counter for NotifyAfterFullLoaded
	if (NotifyAfterFullLoaded && !QueueCounterADAM.Contains(TagName))
	{
		QueueCounterADAM.Add(TagName, 0);
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
			AddToADAM(DataAsset, TagName, RecursiveLoading);
		}
		else
		{
			AddAllToADAM(DataAsset, TagName, RecursiveLoading);
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
	Tag,
	RecursiveLoading);

	// Determine whether the descriptor will be declared and stored
	TSharedPtr<FStreamableHandle> DataAssetHandle = StreamableManager.RequestAsyncLoad(PrimaryDataAsset.ToSoftObjectPath(),	Delegate);
	
	// Adding new data
	if (DataAssetHandle)
	{
		AddDataToArrayADAM(PrimaryDataAsset, DataAssetHandle, Tag);
	}
}

void UAsyncDataAssetManagerSubsystem::AddAllToADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, bool RecursiveLoading)
{
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	// Create a delegate
	FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(
	this,
	&UAsyncDataAssetManagerSubsystem::OnAllLoaded,
	PrimaryDataAsset,
	Tag,
	RecursiveLoading);

	// Determine whether the descriptor will be declared and stored
	TSharedPtr<FStreamableHandle> DataAssetHandle = StreamableManager.RequestAsyncLoad(PrimaryDataAsset.ToSoftObjectPath(),	Delegate);
	
	// Adding new data
	if (DataAssetHandle)
	{
		AddDataToArrayADAM(PrimaryDataAsset, DataAssetHandle, Tag);
	}

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
			Tag,
			false);

	// This handle is not stored in memory
	TSharedPtr<FStreamableHandle> DataAssetHandle = StreamableManager.RequestAsyncLoad(PrimaryDataAsset.ToSoftObjectPath(),	Delegate);
	
	// Return soft reference of Data Asset
	ReturnPrimaryDataAsset = PrimaryDataAsset;
}
