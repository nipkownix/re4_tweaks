![re4_tweaks](https://raw.githubusercontent.com/nipkownix/nipkownix.github.io/master/assets/img/RE4T/re4_t.svg) 

### Description
A project designed to fix and tweak the "UHD" port of Resident Evil 4 on [Steam](https://store.steampowered.com/app/254700/Resident_Evil_4/).

### Current features

 * **Increase FOV** - Increases the default FOV. Useful to prevent motion sickness in some people.

 * **Fix ultrawide aspect ratio** - Fixes the incorrect aspect ratio when playing in ultrawide resolutions, preventing the image from being cut off and the HUD appearing off-screen. 21:9 and 32:9 are supported. (Minor visual bugs may occur in 32:9).

 * **Remove black bars in 16:10** - Removes top and bottom black bars that are present when playing in 16:10. Will crop a few pixels from each side of the screen.

 * **Fix DPI scale** - Forces game to run at normal 100% DPI scaling, fixes resolution issues for players that have above 100% DPI scaling set.

 * **Disable V-Sync** - Forces V-Sync to be disabled. For some reason the vanilla game doesn't provide a functional way to do this.

 * **Fix display mode/black screen issue** - Allows the game to use non-60Hz refresh rates in fullscreen, fixing the black screen issue people have when starting it.

 * **Override laser color** - Overrides the color of the laser sights. Includes bonus "RainbowLaser" option.

 * **Restore pickup screen transparency** - Restores a transparency effect that is missing from the item pickup screeen.
	* Comparison: https://i.imgur.com/YwuC04m.png

 * **Disable broken "Filter03"** - This filter was originally meant to add an extra glow effect on certain fire sources, but it was broken when the game was ported to the Xbox 360, making the entire image have an orange tint overlay applied to it.

 * **Fix blurry image** -  Fixes a problem related to a vertex buffer that caused the image to be slightly blurred, making the image much sharper and clearer.
	* Comparison: https://i.imgur.com/Zv76Uhi.png

 * **Disable film grain** - Disables the film grain overlay that is present in most sections of the game.

 * **Enable GC blur** - Restores DoF blurring from the GC version, which was removed/unimplemented in later ports.

 * **Enable GC scope blur** - Restores outer-scope blurring when using a scope, which was removed/unimplemented in later ports.

 * **Borderless window** - Whether to use a borderless-window when using windowed-mode.

 * **Separate volume sliders** - Allows seperate adjustment of background music/sound effect/cutscene volume.

 * **Mouse turning** - Makes the mouse turn the character instead of controlling the camera, similar to Resident Evil 5.

 * **Raw mouse input** - Makes the game use Raw Input for aiming and turning. Greatly improves mouse input by removing negative/positive accelerations that were being applied both by the game and by Direct Input.

 * **DetachCameraFromAim** - When using the "Modern" mouse setting, the game locks the camera position to the aiming position, making both move together. Although this is the expected behavior in most games, some people might prefer to keep the original camera behavior while also having the benefits from "Modern" aiming. Enabling this will also restore the horizontal aiming sway that was lost when the devs implemented "Modern" aiming.

 * **Fix sniper zoom** - When playing with keyboard and mouse, this fixes an annoying bug which would cause your camera to be off-center after you zoomed in with a sniper rifle.

 * **Fix sniper focus** - When zooming in and out with the sniper rifle using a mouse, this option makes the "focus" animation look similar to how it looks like with a controller.

 * **Fix "Retry/Load" mouse selector** - Prevents the game from overriding your selection in the "Retry/Load" screen when moving the mouse before confirming an action. This bug usually causes people to return to the main menu by mistake, when they actually wanted to just restart from the last checkpoint.

 * **Key Icon Fallback** - Game is missing icons for certain keyboard languages, normally they would be made invisible, but can now have English equivalents shown instead.
 
 * **Allow reload without aiming** - Removes the need to be aiming the weapon before you can reload it. KB/M and Controller supported. (DInput untested). Also provides an extra option to disable the camera zoom when you reload weapons.

 * **Flip items in your inventory using the keyboard** - Creates key bindings to flip (not just rotate) items when playing with a keyboard and mouse, just like you can do if you play with a controller.
    * Default bindings: 
		 * Flip up: Home
		 * Flip down: End
		 * Flip left: Insert
		 * Flip right: Page Up

 * **Jet Ski tricks using keyboard/mouse** - Although a relatively unknown feature, you can do tricks while riding the Jet Ski in the very last section of the game if playing with a controller. This mod adds keyboard/mouse bindings for this, as the devs apparently forgot to do so.

 * **Rebind keyboard QTE keys** - The vanilla game doesn't allow the user to change the main QTE keys, being "X" and "C". Changing them via an .ini file is possible, but the on screen prompts don't change to match the new keys, leading to confusion. This mod changes the default QTE keys to be "A" and "D", both customizable and with proper on screen prompt support.

 * **Change controller sensitivity** - For some reason the vanilla game doesn't have an option to change it for controllers, only for the mouse.

 * **Xinput deadzone changes** - Allows tweaking and removing some unnecessary deadzones that were added for Xinput controllers, making camera movement feel more like it did on the original GameCube version of the game.

 * **4GB Patch** - Can automatically 4GB patch the game executable for you, rather than needing to use tools from other places.

 * **Savegame Transfers** - Removes SteamID check from savegames, allowing saves to be easily transferred between accounts.

 * **60 FPS fixes** - Tries to fix some issues left by the people who created the 60 FPS mode in this port. Currently, it changes the following:
    * Fixes the speed of falling items, making them not fall at double speed. This issue was one of the first issues people noticed (when this port was still being teased in trailers), but Q-LOC never fixed it, even after releasing 6 patches.
	* When running in 60 FPS, some QTEs require extremely fast button presses to work. This gets even worse in Professional difficulty, making it seem almost impossible to survive the minecart and the statue bridge QTEs. This fix makes QTEs that involve rapid button presses much more forgiving.
	* Makes Ashley's bust physics consistent between 30 and 60FPS
	* Makes backwards-turn speed consistent between 30 and 60FPS

 * **Ashley's fixed camera angles** - Unlocks the JP-only classic camera angles during Ashley's segment.

 * **Leon's mafia outfit in cutscenes** - Allows the game to properly display Leon's mafia outfit ("Special 2") in cutscenes.

 * **ViolenceLevelOverride** - Allows overriding the level of violence, making it possible to censor/uncensor the game.

 * **Silent armored Ashley** - Silence Ashley's armored outfit ("Special 2"). For those who also hate the constant "Clank Clank Clank".

 * **AllowAshleySuplex** - Allows Ashley to Suplex enemies in very specific situations. (previously was only possible in the initial NTSC GameCube ver., was patched out in all later ports.)
 
 * **Allow Quickturn when using the Matilda** - Allows quickturning character to camera direction when wielding Matilda.
 
 * **FixDitmanGlitch** - Fixes the Ditman glitch, which would allow players to increase their walk speed. Not recommended, but mod makers may be interested in enabling this (see Bin32/re4_tweaks/setting_overrides/overrides_info.txt)

 * **Sprint toggle** - Changes sprint key to act like a toggle instead of needing to be held.

 * **DisableQTE** - Disables most of the QTEs, making them pass automatically.

 * **AutomaticMashingQTE** - Unlike the previous option, this only automates the "mashing" QTEs, making them pass automatically. Prompts are still shown!

 * **Skip intro logos** - Whether to skip the beginning Capcom etc intro logos when starting up the game.

 * **Enable debug menu** - Enables the "tool menu" debug menu, present inside the game but unused, and adds a few custom menu entries ("SAVE GAME", "DOF/BLUR MENU", etc).
More info in the .ini file.

 * **EnableFastMath** - Replaces older math functions in the game with much more optimized equivalents. Experimental, can hopefully improve framerate in some areas that had dips.
 
 * **PrecacheModels** - Forces game to fully cache all models in the level after loading in. May help with framerate drops when viewing a model for the first time. (not fully tested, could cause issues if level has many models to load!)

 * **Allow high resolution SFD files** - Allocate more memory for SFD movie files, and properly scale its resolution display above 512x336. Not tested beyond 1920x1080.

 * **RaiseVertexAlloc** - Allocate more memory for some vertex buffers. This prevents a crash that can happen when playing with a high FOV.
 
 * **RaiseInventoryAlloc** - Allocate more memory for the inventory screen, preventing crashes with high-poly models inside ss_pzzl.dat.

### Installation

Extract dinput8.dll and dinput8.ini to "Steam\steamapps\common\Resident Evil 4\Bin32".
Options can be configured either by manually editing dinput8.ini, or by pressing F1 to bring up the [in-game configuration menu](https://raw.githubusercontent.com/nipkownix/nipkownix.github.io/master/assets/img/RE4T/cfgMenu.png).

**Linux** users need to add **WINEDLLOVERRIDES=”dinput8=n,b” %command%** into Resident Evil 4’s Steam's launch options.

**If you use Raz0r's "Ultimate Trainer Companion"**, you will notice both mods use the same "dinput8" name. In this case, for them to work together, rename re4_tweaks's files to either "winmm" or "xinput1_3".

### Contributions

Any contributions are welcomed! For those with reverse-engineering experience, we have [an IDA database](https://github.com/emoose/re4-research/issues/1) for the latest version of the game with a huge amount of the games code labelled (based on debug symbols from GC version), has come in very useful for some of the fixes here.

Massive thanks to [emoose](https://github.com/emoose) for his incredible help with many of these features.

Special thanks to MeganGrass for reverse-engineering assistance.

### Support/Problems

In case your game is crashing/failing to load, please try checking the [RE4HD known issues page](https://www.re4hd.com/?p=9454), and follow the file verification guide there - many crashes have been caused by incomplete/corrupt files being used.

If your issue still persists feel free to make a report about it on the [GitHub issues page](https://github.com/nipkownix/re4_tweaks/issues) - if the problem is crashing/loading related you can try creating a `CrashDumps` folder inside the `Bin32` directory (next to `bio4.exe`/`steam_api.dll`/etc), then the next time game crashes hopefully some info about it will be written in there, please include that with any reports you make!
