![Async Data Asset Manager](./_Misc/Preview.png)

# Async Data Asset Manager
ADAM is a plugin for Unreal Engine 5 that adds a subsystem for asynchronous loading and unloading of Data Assets. This subsystem simplifies data management and can be used in both Blueprints and C++.

<br>

> [!NOTE]
> The plugin has been pre-packaged only for Win64 and Android.

## Latest Updates
`Version 1.4.3`
- Build version for Unreal Engine 5.6.0+
- Improved recursive loading option. The function parameters of `LoadADAM` and `LoadArrayADAM` have been updated. Added the ability to specify the recursion depth, allowing data to be loaded only up to a defined level of nesting.
- `New` Added a new function `GetDataByClassADAM`. This function retrieves a filtered list of loaded data assets from ADAM’s internal memory based on class and tag. (For example, this can be useful when working with deeply nested assets required for data retrieval.)

## What it's for
- Load and unload Data Assets asynchronously using simple functions.
- Easily and quickly configure your logic for data management.

## Features
- Fast and simple management of asynchronous Data Asset loading without the need to use C++ code.
- The ADAM subsystem supports parallel asynchronous loading of Data Assets from multiple sources, controlling random duplicate load and unload requests in real-time.
- Additional duplicate checking ensures that there are no additional references to resources in memory and that they are retained by the standard system.
- Supports bulk asynchronous loading of unique Data Assets.
- This subsystem enables recursive data loading. If you load a single DataAsset that includes multiple nested Data Assets, all of them will be loaded and filtered to avoid duplicates in memory.
- Group your uploaded DataAssets using tags so that they can be unloaded at the right moment <i>(for example, this can be useful if you are uploading DataAssets in parts and want to unload them without affecting other necessary data still stored in memory)</i>. You can also choose which approach to use: a regular `FName` for complex tags (e.g., level names) or fixed tags like `FGameplayTag` for centralized management (so you don’t have to keep everything written down).
- Supports asynchronous loading without memory retention <i>(e.g., if you need to immediately access data and then free up memory)</i>.
- Single notification for bulk data load. The `OnAllLoadedADAM` delegate notifies when all Data Assets have been loaded simultaneously. It only functions if the `NotifyAfterFullLoaded` option is enabled, which is supported exclusively by the `LoadArrayADAM` method.
- Disableable debug logs allow you to monitor the entire asynchronous data management process. Plugin settings are located in `Project Settings > Plugins > Async Technologies - ADAM`.

## Install
1. Make sure the Unreal Engine editor is closed.
2. Move the "Plugins" folder to the root folder of your created project.
3. Run your project to which the "Plugins" folder with 'AsyncDataAssetManager' was added. If a message about restoring the module appears, select "Yes".
4. Done! The 'Async Data Asset Manager' folders should appear in the Unreal Engine browser and the plugin should be automatically activated. If the plugin folder is not visible, activate visibility through the browser settings: `Settings > Show Plugin Content`.

## How to use it?
An interactive step-by-step tutorial on how to use ADAM can be found in the file: `B_GameMode_DemoADAM`, which is located at the path `Plugins\Async Data Asset Manager Content\DemoFiles\`.

![Window Manager](./_Misc/Tutorial/Tutorial_1.jpg)
![Window Manager](./_Misc/Tutorial/Tutorial_2.jpg)
![Window Manager](./_Misc/Tutorial/Tutorial_3.jpg)
![Window Manager](./_Misc/Tutorial/Tutorial_4.jpg)

## (C++) Documentaion
All sources contain self-documenting code.
