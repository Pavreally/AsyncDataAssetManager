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
 * *** Adding a progress bar ***
 * If you want to get the percentage of loading completion, you can use the 'QueueADAM' 
 * array. This array stores the real-time queue for asynchronous loading, which is 
 * performed using the 'LoadADAM' and 'LoadArrayADAM' functions. When the data is 
 * loaded, this array is cleared after the 'FOnLoadedADAM OnLoadedADAM' delegate is triggered.
 * 
 * *** Recursive Asynchronous Loading ***
 * This subsystem enables recursive data loading. If you load a single DataAsset that 
 * includes multiple nested DataAssets, all of them will be loaded and filtered to 
 * avoid duplicates in memory. Additionally, if you change a tag, the entire data 
 * package will share the specified tag.
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ADAM Subsystem")
	FString PrimaryDataAssetName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ADAM Subsystem")
	TSoftObjectPtr<UPrimaryDataAsset> SoftReference;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ADAM Subsystem")
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
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

#pragma region DELEGATES
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnLoadedADAM, UPrimaryDataAsset*, LoadedObject, TSoftObjectPtr<UPrimaryDataAsset>, LoadedPrimaryDataAsset, FName, LoadedTag, bool, RecursiveLoading);

	// Indicates that the download is complete
	UPROPERTY(BlueprintAssignable, Category = "ADAM Subsystem")
	FOnLoadedADAM OnLoadedADAM;

#pragma endregion DELEGATES

	/**
	 * The main array of the ADAM subsystem. 
	 * Suljit as a soft link store for all asynchronously loaded data assets.
	 */
	TArray<FMemoryADAM> DataADAM;

	// Auxiliary array responsible for the safety of parallel asynchronous loading in real-time.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ADAM Subsystem", meta = (ToolTip = "Auxiliary array responsible for the safety of parallel asynchronous loading in real-time."))
	TArray<FString> QueueADAM;

#pragma region BlueprintFunctions
	/**
	 * Async loading of a Data Asset and storing it in memory.
	 * @param Tag Designed for data grouping.
	 * @param RecursiveLoading Support for recursion. During loading, the function will check for nested Data Assets and attempt to load them. Warning! This option may require more performance and processing time.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	void LoadADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, bool RecursiveLoading, TSoftObjectPtr<UPrimaryDataAsset>& ReturnPrimaryDataAsset);

	/**
	 * Async loading of an array of Data Asset and storing each element in memory.
	 * @param Tag Designed for data grouping.
	 * @param RecursiveLoading Support for recursion. During loading, the function will check for nested Data Assets and attempt to load them. Warning! This option may require more performance and processing time.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	void LoadArrayADAM(TArray<TSoftObjectPtr<UPrimaryDataAsset>> PrimaryDataAssets, FName Tag, bool RecursiveLoading, TArray<TSoftObjectPtr<UPrimaryDataAsset>>& ReturnPrimaryDataAssets);

	// Loading a Data Asset without storing it in memory.
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	void FastLoadADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, TSoftObjectPtr<UPrimaryDataAsset>& ReturnPrimaryDataAsset);

	/**
	 * Unload one Data Asset from array and memory.
	 * @param ForcedUnload If true, the function call will stop loading the Data Asset
	 * asynchronously and will make the target resource available for memory freeing,
	 * on the next rubbish collection. If false, the function call will immediately clear memory from the target resource.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	void UnloadADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, bool ForcedUnload);

	/**
	 * Unload all Data Assets from array and memory. Unloading in descending order.
	 * @param ForcedUnload If true, the function call will stop loading the Data Asset
	 * asynchronously and will make the target resource available for memory freeing,
	 * on the next rubbish collection. If false, the function call will immediately clear memory from the target resource.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	void UnloadAllADAM(bool ForcedUnload);

	/**
	 * Unload all data assets with the specified tag from the array and memory. Unloading in descending order.
	 * @param Tag Alternative deletion option. If this value is changed, all Data Assets with the specified tag will be removed from memory.
	 * @param ForcedUnload If true, the function call will stop loading the Data Asset
	 * asynchronously and will make the target resource available for memory freeing,
	 * on the next rubbish collection. If false, the function call will immediately clear memory from the target resource.
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
	 * @param IsValid Returns the validity state of the object.
	 */
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	UObject* GetObjectDataADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, bool& IsValid);

	// Returns the index from the array of the ADAM data asset collection. If nothing is found returns -1.
	UFUNCTION(BlueprintCallable, Category = "ADAM Subsystem")
	int32 GetIndexDataADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset);
	
#pragma endregion BlueprintFunctions

protected:
	UPROPERTY()
	bool EnableLog = false;

	// Events executed after initialization.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Post Subsystem Init", ToolTip = "Executes code after system initialization."))
	void ReceivePostSubsystemInit();

	UFUNCTION()
	void RemoveFromADAM(int32 DataAssetIndex, bool ForcedUnload);

	// Single asynchronous loading with completion notification
	UFUNCTION()
	void AddToADAM(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, bool RecursiveLoading);

	// Recursive loading using AddToADAM
	UFUNCTION()
	void RecursiveLoad(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag);

	UFUNCTION()
	void OnLoaded(TSoftObjectPtr<UPrimaryDataAsset> PrimaryDataAsset, FName Tag, bool RecursiveLoading);

private:
	UFUNCTION()
	TArray<TSoftObjectPtr<UPrimaryDataAsset>> FindNestedAssets(UPrimaryDataAsset* DataAsset);
};
