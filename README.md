# Personal Template Starter Files for Raylib Projects
This is a starter filer for raylib projects. Targets only MacOS and Windows (no linux support for now).
It is a simple unity build so no additional translation units.
It is meant to be a simple starter point with enough simple QoL features
Feel free to ping me for questions where things are unclear. It is very much a tool built for my quick and dirty projects and experiments. So I haven't spent a lot of time documenting it since this is still an evovling template.

## Current features
- Uses Tsoding's nob.h project as the build system https://github.com/tsoding/nob.h so everything is 100% in c, you just need to c compiler.
  - To start, just `cc -o nob nob.h` and then run the respective `nob` or `nob.exe` to build. This does the basic debug build.
  - `nob release` to build an optimised, non debug version of the build. Where DEBUG=0 (debug builds have DEBUG=1 flag).
  - `nob clean` to ONLY clean the build folder
  - `nob spritepack` to ONLY build sprite atlases
- Has debug support in vscode via the relevant json files in `/.vscode`
- Has sprite packing functionality built-in. Examples can be seen in main
  - You can add new atlases via `build_defines.h` and change the max number of sprites for each atlas, along with the atlas size.
  - New atlasses must be called in the `nob.c`'s `execute_cmd_sprite_packer` function. To build a sprite atlas, you must add to here the `build_texture_atlas` call with the newly defined atlas names, max sprites, and size.
  - You also need to create a folder inside `/resources` with the atlas name, to be passed to the `build_texture_atlas` call.
  - Each sprite in the atlas must be added to `game_defines.h` SpriteID, and the atlas itself added to TextureID
  - Lastly, you need to add to `texture_packer_utils.h` in the `LoadAllTexturesAndSprites` function the actual call to load the texture and its metadata into usable SpriteData.
  - Refer to mainAtlas as an example, and see also `main.c`'s usage of the atlas to draw the player sprites.
- Has consumable input support.
  - `pollInputs` is called in main, which polls into a consumableInput struct that is zeroed every start of the frame.
  - Use `tryConsumeInput` and `peakInput` among other functions that you can find in input_utils.h

## Additional Settings
### Renaming the Project
- To rename the project you need to change the OUT_PATH in nob.c
  - If you are using vscode, change the "outputNameNoExtension" in `/.vscode/settings.json`

## Future TODO considerations
### Known Issues
- Not all gamepads are supported on MacOS due to a known upstream problem of GLFW on mac: https://github.com/raysan5/raylib/issues/3651
  - This is possibly fixed using SDL as the platform backend for raylib, however, even with SDL remapping / testing kits it seems some of the non-working controllers still have issues.
  - Additionally, dug through some of raylib's src files and found that the SDL Platform is only tested for Windows and Linux, but not mac, hence it would not solve the problem on mac.

### Possible Inclusions
- Other projects of mine have a player input log and replay feature. Looking into including it. But weighing it it is worth the complexity as such replay systems are rather fine tuned to each project.
