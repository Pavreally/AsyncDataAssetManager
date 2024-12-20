// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/StreamableManager.h"

#include "AsyncDataAssetManagerSubsystem.generated.h"

class UPrimaryDataAsset;

/**
 * Async Data Asset Manager Subsystem (ADAM Subsystem)
 * 
 * *** Description ***
 * This subsystem is designed to streamline operations and expedite the setup 
 * of projects for the asynchronous loading and unloading of Primary Data Assets. 
 * ADAM leverages the StreamableManager system for asynchronous loading and can 
 * also independently store information about the loaded data in the form of links and tags.
 * 
 * *** Recursive Asynchronous Loading ***
 * This subsystem enables recursive data loading. If you load a single DataAsset that 
 * includes multiple nested DataAssets, all of them will be loaded and filtered to 
 * avoid duplicates in memory. Additionally, if you change a tag, the entire data 
 * package will share the specified tag.
 * 
 * *** Only for the array load function ***
 * The "Notify After Full Loaded" option, together with the "On All Loaded" event, 
 * allows you to receive a notification only when all specified data has been loaded.
 * 
 */

#pragma region STRUCTS
// The main structure of the ADAM subsystem.
USTRUCT()
struct FMemoryADAM
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	TSoftObjectPtr<UPrimaryDataAsset> SoftReference;
	
	TSharedPtr<FStreamableHandle> MemoryReference;

	UPROPERTY()
	FName Tag;
};

// Designed to output information about the current storage of data assets.
USTRUCT(BlueprintType)
struct FMirrorADAM
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ADAM Subsystem", meta = (ToolTip = "Data asset name."))
	FString PrimaryDataAssetName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ADAM Subsystem", meta = (ToolTip = "Soft link to data asset."))
	TSoftObjectPtr<UPrimaryDataAsset> SoftReference;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ADAM Subsystem", meta = (ToolTip = "Designed for data grouping."))
	FName Tag;
};

#pragma endregion STRUCTS

/**
 * ADAM subsystem main class
 */
UCLASS(Blueprintable)
class ASYNCDATAASSETMANAGER_API UAsyncDataAssetManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	//~USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End USubsystem

#pragma region DELEGATES
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnLoadedADAM, UPrimaryDataAsset*, LoadedObject, TSoftObjectPtr<UPrimaryDataAsset>, LoadedPrimaryDataAsset, FName, LoadedTag, bool, RecursiveLoading);

	// Indicates that the load is complete
	UPROPERTY(BlueprintAssignable, Category = "ADAM Subsystem")
	FOnLoadedADAM OnLoadedADAM;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAllLoadedADAM, FName, LoadedTag);

	// Indicates that the load is complete
	UPROPERTY(BlueprintAssignable, Category = "ADAM Subsystem")
	FOnAllLoadedADAM OnAllLoadedADAM;

#pragma endregion DELEGATES

	/**
	 * The main array of the ADAM subsystem. 
	 * Suljit as a soft link store for all asynchronously loaded data assets.
	 */
	TArray<FMemoryADAM> DataADAM;

	// Auxiliary array responsible for the safety of parallel asynchronous loading in real-time.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ADAM Subsystem", meta = (ToolTip = "Auxiliary array responsible for the safety of parallel asynchronous loading in real-time."))
	TArray<FString> QueueADAM;

#pragma region BLUEPRINT_FUNCTIONS
	/**
	 * Async loading of a Data Asset and storing it in memory.
	 * @param PrimaryDataAsset Soft link to data asset.
	 * @param Tag Designed for data grouping.
	 * @param RecursiveLoading Support for recursion. During loading, the function will check for nested Data Assets and attempt to load them. Warning! This option may require more performance and processing time.
	 * @return ReturnPrimaryDataAsset - Returns the same data asset as that specified in the first parameter.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	void LoadADAM(
			TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset,
			FName Tag, 
			bool RecursiveLoading, 
			TSoftObjectPtr<UPrimaryDataAsset>& ReturnPrimaryDataAsset);

	/**
	 * Async loading of an array of Data Asset and storing each element in memory.
	 * @param PrimaryDataAssets Soft link to data assets.
	 * @param Tag Designed for data grouping.
	 * @param NotifyAfterFullLoaded If true, the "OnAllLoaded" event will notify you when all data in the array has been fully loaded. The ADAM system will ignore duplicate checks (to prevent accidental unloading of necessary data through another thread), so all Data Asset duplicates will be controlled by the engine's base system.
	 * @param RecursiveLoading Support for recursion. During loading, the function will check for nested Data Assets and attempt to load them. Warning! This option may require more performance and processing time.
	 * @result ReturnPrimaryDataAssets - Returns the same data asset as that specified in the first parameter.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	void LoadArrayADAM(
			TArray<TSoftObjectPtr<UPrimaryDataAsset>> PrimaryDataAssets, 
			FName Tag, 
			bool NotifyAfterFullLoaded,
			bool RecursiveLoading, 
			TArray<TSoftObjectPtr<UPrimaryDataAsset>>& ReturnPrimaryDataAssets);

	/**
	 * Loading a Data Asset without storing it in memory.
	 * 
	 * @param PrimaryDataAsset Soft link to data asset.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	void FastLoadADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, TSoftObjectPtr<UPrimaryDataAsset>& ReturnPrimaryDataAsset);

	/**
	 * Unload one Data Asset from array and memory.
	 * 
	 * @param PrimaryDataAsset Soft link to data asset.
	 * @param ForcedUnload If false, the function call will stop loading the Data Asset
	 * asynchronously and will make the target resource available for memory freeing,
	 * on the next rubbish collection. If true, the function call will immediately clear memory from the target resource.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	void UnloadADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, bool ForcedUnload);

	/**
	 * Unload all Data Assets from array and memory. Unloading in descending order.
	 * @param ForcedUnload If false, the function call will stop loading the Data Asset
	 * asynchronously and will make the target resource available for memory freeing,
	 * on the next rubbish collection. If true, the function call will immediately clear memory from the target resource.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	void UnloadAllADAM(bool ForcedUnload);

	/**
	 * Unload all data assets with the specified tag from the array and memory. Unloading in descending order.
	 * @param Tag Alternative deletion option. If this value is changed, all Data Assets with the specified tag will be removed from memory.
	 * @param ForcedUnload If false, the function call will stop loading the Data Asset
	 * asynchronously and will make the target resource available for memory freeing,
	 * on the next rubbish collection. If true, the function call will immediately clear memory from the target resource.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	void UnloadAllTagsADAM(FName Tag, bool ForcedUnload);

	/**
	 * Returns the ADAM mirror array formed it the main array with a reference to the destructor.
	 * @return Returns a mirrored array of DataADAM.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	TArray<FMirrorADAM> GetDataADAM();

	/**
	 * Returns an array that shows how many data items there are in the ADAM system for each unique tag.
	 * @return Returns the tag values and the corresponding number of Data Assets.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	TMap<FName, int32> GetCollectionByTagADAM();

	/**
	 * Returns a pointer to the Data Asset ADAM object stored in memory.
	 * 
	 * @param PrimaryDataAsset Soft link to data asset.
	 * @param IsValid Returns the validity state of the object.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	UObject* GetObjectDataADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, bool& IsValid);

	/**
	 * Returns the index from the array of the ADAM data asset collection. If nothing is found returns -1.
	 * 
	 * @param PrimaryDataAsset Soft link to data asset.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	int32 GetIndexDataADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset);
	
#pragma endregion BLUEPRINT_FUNCTIONS

protected:

	/**
	 * Remove Data Asset from the ADAM array and asynchronously unload it.
	 * 
	 * @param ForcedUnload If false, the function call will stop loading the Data Asset
	 */
	UFUNCTION()
	void RemoveFromADAM(int32 DataAssetIndex, bool ForcedUnload);

	/**
	 * Single asynchronous loading with completion notification
	 * 
	 * @param PrimaryDataAsset Soft link to data asset.
	 * @param Tag Designed for data grouping.
	 * @param RecursiveLoading Support for recursion. During loading, the function will check for nested Data Assets and attempt to load them. Warning! This option may require more performance and processing time.
	 */
	UFUNCTION()
	void AddToADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, bool RecursiveLoading);

	/**
	 * Multiple asynchronous loading with completion notification
	 * 
	 * Unlike the AddToADAM() function, this function allows loading the same Data Asset
	 * using FStreamableManager. If the Data Asset is already being loaded or is loaded,
	 * FStreamableManager will not load it again. Instead, it will increase the reference
	 * count for this resource. When all references to the resource are removed, 
	 * FStreamableManager will free the memory occupied by this resource.
	 * 
	 * @param PrimaryDataAsset Soft link to data asset.
	 * @param Tag Designed for data grouping.
	 * @param RecursiveLoading Support for recursion. During loading, the function will check for nested Data Assets and attempt to load them. Warning! This option may require more performance and processing time.
	 */
	UFUNCTION()
	void AddAllToADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, bool RecursiveLoading);

	/**
	 * Asynchronous recursive loading of Data Asset
	 * 
	 * @param PrimaryDataAsset Soft link to data asset.
	 * @param Tag Designed for data grouping.
	 * @param NotifyAfterFullLoaded If true, the "OnAllLoaded" event will notify you when all data in the array has been fully loaded. The ADAM system will ignore duplicate checks (to prevent accidental unloading of necessary data through another thread), so all Data Asset duplicates will be controlled by the engine's base system.
	 */
	UFUNCTION()
	void RecursiveLoad(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, bool NotifyAfterFullLoaded);

	/**
	* Delegate notification after loading Data Asset into ADAM subsystem
	* 
	* @param PrimaryDataAsset Soft link to data asset.
	* @param Tag Designed for data grouping.
	* @param RecursiveLoading Support for recursion. During loading, the function will check for nested Data Assets and attempt to load them. Warning! This option may require more performance and processing time.
	* 
	* LoadedObject - the loaded data asset object.
	* PrimaryDataAsset - a soft link with a generic suffix.
	* Tag - a given tag for grouping data.
	* RecursiveLoading - whether the recursive option was selected during loading.
	*/
	UFUNCTION()
	void OnLoaded(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, bool RecursiveLoading);

	/**
	 * Delegate notification after full loading Data Asset into ADAM subsystem
	 * 
	 * @param PrimaryDataAsset Soft link to data asset.
	 * @param Tag Designed for data grouping.
	 * @param RecursiveLoading Support for recursion. During loading, the function will check for nested Data Assets and attempt to load them. Warning! This option may require more performance and processing time.
	 * 
	 * LoadedObject - the loaded data asset object.
	 * PrimaryDataAsset - a soft link with a generic suffix.
	 * Tag - a given tag for grouping data.
	 * RecursiveLoading - whether the recursive option was selected during loading.
	 */
	UFUNCTION()
	void OnAllLoaded(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, bool RecursiveLoading);

private:

	UPROPERTY()
	bool EnableLog = false;

	UPROPERTY()
	TMap<FName, int32> QueueCounterADAM;

	// Function for searching nested data assets
	UFUNCTION()
	TArray<TSoftObjectPtr<UPrimaryDataAsset>> FindNestedAssets(UPrimaryDataAsset* DataAsset);

	/**
	 * Add data to the main DataADAM array
	 * @param PrimaryDataAsset Soft link to data asset.
	 * @param DataAssetHandle Data Asset Descriptor.
	 * @param Tag Designed for data grouping.
	 */
	void AddDataToArrayADAM(
			TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset,
			TSharedPtr<FStreamableHandle> DataAssetHandle,
			FName Tag);
};
