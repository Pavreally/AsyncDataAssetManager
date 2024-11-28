![Async Data Asset Manager](./_Misc/Preview.png)

# Async Data Asset Manager
`BETA`<br><br>
ADAM is a plugin for Unreal Engine 5 that adds a subsystem for asynchronous loading and unloading of Data Assets. This subsystem simplifies data management and can be used in both Blueprints and C++.

<br>
> [!NOTE]
> The plugin has been pre-packaged only for Win64 and Android.

# Latest Updates
`Version 1.1`
- Build version for Unreal Engine 5.5.
- Refactoring and optimization of the code.
- Enhanced security for memory resource management.
- Added the ability to specify a tag for uploaded files.
- Added a new function `UnloadAllTagsADAM`, which will unload DataAssets stored in memory by the specified tag.
- Added a new function `GetCollectionByTagADAM`, which will display a list of tags stored in the ADAM system (including default tags - NONE) and their total count.

## What it's for
- Load and unload Data Assets asynchronously using simple functions.
- Easily and quickly configure your logic for data management.

## Features
- Fast and simple management of asynchronous Data Asset loading without the need to use C++ code.
- The ADAM subsystem supports parallel asynchronous loading of Data Assets from multiple sources, controlling random duplicate load and unload requests in real-time.
- Supports bulk asynchronous loading of unique Data Assets.
- Group your uploaded DataAssets using tags so that they can be unloaded at the right moment <i>(for example, this can be useful if you are uploading DataAssets in parts and want to unload them without affecting other necessary data still stored in memory)</i>.
- Supports asynchronous loading without memory retention (e.g., if you need to immediately access data and then free up memory).
- Disableable debug logs allow you to monitor the entire asynchronous data management process. Plugin settings are located in `Project Settings > Plugins > Async Technologies - ADAM`.

## Install
1. Make sure the Unreal Engine editor is closed.
2. Move the "Plugins" folder to the root folder of your created project.
3. Run your project to which the "Plugins" folder with 'AsyncDataAssetManager' was added. If a message about restoring the module appears, select "Yes".
4. Done! The 'Async Data Asset Manager' folders should appear in the Unreal Engine browser and the plugin should be automatically activated. If the plugin folder is not visible, activate visibility through the browser settings: `Settings > Show Plugin Content`.

## How to use it?
An interactive step-by-step tutorial on how to use ADAM can be found in the file: `BP_GameMode_ADAM_demo`, which is located at the path `Plugins\Async Data Asset Manager Content\DemoFiles\`.

![Window Manager](./_Misc/Tutorial/Tutorial_1.jpg)
![Window Manager](./_Misc/Tutorial/Tutorial_2.jpg)
![Window Manager](./_Misc/Tutorial/Tutorial_3.jpg)

## (C++) Documentaion
All sources contain self-documenting code.