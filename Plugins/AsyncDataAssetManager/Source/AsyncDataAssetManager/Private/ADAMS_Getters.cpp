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

	// Use a set for faster unique checks
	TSet<FString> UniqueAssetNames;

	// Start recursion from the DataAsset itself
	FindNestedAssetsRecursive(DataAsset, DataAsset->GetClass(), NestedAssets, UniqueAssetNames);

	if (EnableLog && NestedAssets.Num() == 0)
	{
		UE_LOG(LogTemp, Display, TEXT("ADAM (Recursive data): Iteration is complete. All nested data is loaded!"));
	}

	return NestedAssets;
}

void UAsyncDataAssetManagerSubsystem::FindNestedAssetsRecursive(void* Container, UStruct* Struct, TArray<TSoftObjectPtr<UPrimaryDataAsset>>& OutNestedAssets, TSet<FString>& UniqueAssetNames)
{
	if (!Container || !Struct) 
		return;

	// Iterate over all properties in the struct/class
	for (TFieldIterator<FProperty> PropIterator(Struct); PropIterator; ++PropIterator)
	{
		const FProperty* Prop = *PropIterator;
		// Handle soft object properties (TSoftObjectPtr<UPrimaryDataAsset>)
		if (const FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Prop))
		{
			if (SoftObjectProperty->PropertyClass->IsChildOf(UPrimaryDataAsset::StaticClass()))
			{
				TSoftObjectPtr<UPrimaryDataAsset> ChildAsset = *SoftObjectProperty->ContainerPtrToValuePtr<TSoftObjectPtr<UPrimaryDataAsset>>(Container);
				FString ChildAssetName = ChildAsset.GetAssetName();
				if (!ChildAssetName.IsEmpty() && !UniqueAssetNames.Contains(ChildAssetName))
				{
					UniqueAssetNames.Add(ChildAssetName);
					OutNestedAssets.Add(ChildAsset);
				}
			}
		}
		// Handle struct properties (recurse into the struct)
		else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Prop))
		{
			void* StructData = StructProperty->ContainerPtrToValuePtr<void>(Container);
			if (StructData)
			{
				FindNestedAssetsRecursive(StructData, StructProperty->Struct, OutNestedAssets, UniqueAssetNames);
			}
		}
		// Handle array properties
		else if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(Prop))
		{
			FScriptArrayHelper ArrayHelper(ArrayProp, ArrayProp->ContainerPtrToValuePtr<void>(Container));
			// Check inner property type
			const FProperty* InnerProp = ArrayProp->Inner;
			// If inner is soft object ptr
			if (const FSoftObjectProperty* InnerSoftObjectProperty = CastField<FSoftObjectProperty>(InnerProp))
			{
				if (InnerSoftObjectProperty->PropertyClass->IsChildOf(UPrimaryDataAsset::StaticClass()))
				{
					for (int32 i = 0; i < ArrayHelper.Num(); ++i)
					{
						TSoftObjectPtr<UPrimaryDataAsset> ChildAsset = *reinterpret_cast<TSoftObjectPtr<UPrimaryDataAsset>*>(ArrayHelper.GetRawPtr(i));
						FString ChildAssetName = ChildAsset.GetAssetName();
						if (!ChildAssetName.IsEmpty() && !UniqueAssetNames.Contains(ChildAssetName))
						{
							UniqueAssetNames.Add(ChildAssetName);
							OutNestedAssets.Add(ChildAsset);
						}
					}
				}
			}
			// If inner is struct, recurse on each array element
			else if (const FStructProperty* InnerStructProperty = CastField<FStructProperty>(InnerProp))
			{
				for (int32 i = 0; i < ArrayHelper.Num(); ++i)
				{
					void* ElementData = ArrayHelper.GetRawPtr(i);
					if (ElementData)
					{
						FindNestedAssetsRecursive(ElementData, InnerStructProperty->Struct, OutNestedAssets, UniqueAssetNames);
					}
				}
			}
		}
	}
}

TArray<TSoftObjectPtr<UPrimaryDataAsset>> UAsyncDataAssetManagerSubsystem::GetDataByClassADAM(TSubclassOf<UPrimaryDataAsset> DataAssetClass, FName Tag, bool bIgnoreTag)
{
	TArray<TSoftObjectPtr<UPrimaryDataAsset>> SortedPrimaryDataAsset;

	if (!DataAssetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADAM (GetDataByClass): DataAssetClass is nullptr!"));

		return SortedPrimaryDataAsset;
	}

	for (const FMemoryADAM& Data : DataADAM)
	{
		if (Data.MemoryReference.IsValid() && Data.SoftReference->GetClass()->IsChildOf(DataAssetClass))
		{
			if (bIgnoreTag || Data.Tag == Tag)
			{
				SortedPrimaryDataAsset.Add(Data.SoftReference);
			}
		}
	}

	return SortedPrimaryDataAsset;
}
