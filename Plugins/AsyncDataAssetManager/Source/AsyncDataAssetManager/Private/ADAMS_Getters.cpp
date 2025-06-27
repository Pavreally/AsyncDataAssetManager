// Pavel Gornostaev <https://github.com/Pavreally>

#include "AsyncDataAssetManagerSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/DataAsset.h"
#include "AsyncTechnologiesSettings.h"

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
