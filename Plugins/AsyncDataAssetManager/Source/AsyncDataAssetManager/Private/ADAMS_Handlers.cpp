// Pavel Gornostaev <https://github.com/Pavreally>

#include "AsyncDataAssetManagerSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/DataAsset.h"
#include "AsyncTechnologiesSettings.h"

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

FName UAsyncDataAssetManagerSubsystem::GetTagNameFromStruct(FTagADAM& Tag)
{
	if (Tag.GameplayTag.IsValid())
	{
		return Tag.GameplayTag.GetTagName();
	}
	else if (!Tag.TagName.IsNone())
	{
		return Tag.TagName;
	}

	return NAME_None;
}
