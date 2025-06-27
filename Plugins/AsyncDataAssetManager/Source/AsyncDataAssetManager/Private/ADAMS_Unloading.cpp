// Pavel Gornostaev <https://github.com/Pavreally>

#include "AsyncDataAssetManagerSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/DataAsset.h"
#include "AsyncTechnologiesSettings.h"

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

void UAsyncDataAssetManagerSubsystem::UnloadAllTagsADAM(FTagContainerADAM Tag, bool ForcedUnload)
{
	if (DataADAM.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Unload All Tags ADAM): Nothing to delete. The \"DataADAM\" array is empty."));

		return;
	}

	TArray<FName> TagNameContainerCache;

	// Determines the type of string and returns it to the FName array.
	if (!Tag.GameplayTags.IsEmpty())
	{
		TArray<FGameplayTag> GameplayTagContainer = Tag.GameplayTags.GetGameplayTagArray();
		// Convert tags to FName.
		for (FGameplayTag GameplayTag : GameplayTagContainer)
		{
			TagNameContainerCache.Add(GameplayTag.GetTagName());
		}
	}
	
	if (Tag.TagNameContainer.Num() != 0)
	{
		TagNameContainerCache.Append(Tag.TagNameContainer);
	}
	
	if (!Tag.TagName.IsNone())
	{
		TagNameContainerCache.Add(Tag.TagName);
	}

	// Set default value if no other data
	if (TagNameContainerCache.Num() == 0)
	{
		TagNameContainerCache.Add(NAME_None);
	}

	// Remove of all data with a similar target tag
	for (FName TagName : TagNameContainerCache)
	{
		for (int32 i = DataADAM.Num() - 1; i >= 0; i--)
		{
			if (DataADAM[i].Tag == TagName)
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
		UE_LOG(LogTemp, Warning, TEXT("ADAM (Remove From ADAM): Invalid index [%d] for data clearing."), DataAssetIndex);
		
		return;
	}
}
