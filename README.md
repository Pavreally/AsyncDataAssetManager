![Async Data Asset Manager](./_Misc/Preview.png)

# Async Data Asset Manager
ADAM is a plugin for Unreal Engine 5 that adds a subsystem for asynchronous loading and unloading of Data Assets. This subsystem simplifies data management and can be used in both Blueprints and C++.

<br>

> [!NOTE]
> The plugin has been pre-packaged only for Win64 and Android.

## Latest Updates
`Version 1.4.1`
- Build version for Unreal Engine 5.6.0
- Code refactoring and decomposition.
- Improving code security.
- Cleaning up the chaos in the DemoFiles folder after the latest patch and putting things in order.
- Redesign of the data asset loading tag system. Tags in the ADAM subsystem functions are now universal — this means that for each data asset package, you can choose which approach to use: a regular `FName` for complex tags (e.g., level names) or fixed tags like `FGameplayTag` for centralized management (so you don’t have to keep everything written down). Both approaches can be used at the same time, since the main storage supports both tag types as `FName`. However, only one tag type can be used per data asset package.
- Redesign of the data asset unloading tag system in the `UnloadAllTagsADAM` function. For convenience, the tag selection has been expanded to three. You can fill in and specify all the tag types you need to unload at once.
- Correction for the `UnloadAllTagsADAM` function — now it is also possible to bulk unload from memory only data assets with the tag "None".

## What it's for
- Load and unload Data Assets asynchronously using simple functions.
- Easily and quickly configure your logic for data management.

## Features
- Fast and simple management of asynchronous Data Asset loading without the need to use C++ code.
- The ADAM subsystem supports parallel asynchronous loading of Data Assets from multiple sources, controlling random duplicate load and unload requests in real-time.
- Additional duplicate checking ensures that there are no additional references to resources in memory and that they are retained by the standard system.
- Supports bulk asynchronous loading of unique Data Assets.
- This subsystem enables recursive data loading. If you load a single DataAsset that includes multiple nested Data Assets, all of them will be loaded and filtered to avoid duplicates in memory.
- Group your uploaded DataAssets using tags so that they can be unloaded at the right moment <i>(for example, this can be useful if you are uploading DataAssets in parts and want to unload them without affecting other necessary data still stored in memory)</i>.
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

## (C++) Documentaion
All sources contain self-documenting code.
