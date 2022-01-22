
# re4_tweaks

### Description
A project designed to fix and tweak the "UHD" port of Resident Evil 4 on [Steam](https://store.steampowered.com/app/254700/Resident_Evil_4/).

### Current features

 * **Increase FOV** - Increases the default FOV. Useful to prevent motion sickness in some people.

 * **Fix ultrawide aspect ratio** - Fixes the incorrect aspect ratio when playing in ultrawide resolutions, preventing the image from being cut off and the HUD appearing off-screen. Only tested in 21:9.

 * **Fix vsync toggle** - Makes it so that the vsync option found in the game's config.ini actually works.

 * **Restore pickup screen transparency** - Restores a transparency effect that was present on the GameCube and Wii versions of RE4, but has been missing since the game was ported to Xbox 360. 
	* Comparison: https://i.imgur.com/YwuC04m.png

 * **Disable broken "Filter03"** - This filter was originally meant to add an extra glow effect on certain fire sources, but it was broken when the game was ported to the Xbox 360, making the entire image have an orange tint overlay applied to it.

 * **Fix blurry image** -  Fixes a problem related to a vertex buffer that caused the image to be slightly blurred, making the image much sharper and clearer.
	* Comparison: https://i.imgur.com/Zv76Uhi.png

 * **Disable film grain** - Disables the film grain overlay that is present in most sections of the game.

 * **Enable GC blur** - Restores DoF blurring from the GC version, which was removed/unimplemented in later ports.

 * **Enable GC scope blur** - Restores outer-scope blurring when using a scope, which was removed/unimplemented in later ports.

 * **Borderless window** - Whether to use a borderless-window when using windowed-mode.

 * **Fix QTE** - When running in 60 FPS, some QTEs require extremely fast button presses to work. This gets even worse in Professional difficulty, making it seem almost impossible to survive the minecart and the statue bridge QTEs. This fix makes QTEs that involve rapid button presses much more forgiving.

 * **Skip intro logos** - Whether to skip the beginning Capcom etc intro logos when starting up the game.

 * **Enable debug menu** - Enables the "tool menu" debug menu, present inside the game but unused, and adds a few custom menu entries ("SAVE GAME", "DOF/BLUR MENU", etc).
More info in the .ini file.

 * **Fix sniper zoom** - When playing with keyboard and mouse, this fixes an annoying bug which would cause your camera to be off-center after you zoomed in with a sniper rifle.

 * **Fix "Retry/Load" mouse selector** - Prevents the game from overriding your selection in the "Retry/Load" screen when moving the mouse before confirming an action. This bug usually causes people to return to the main menu by mistake, when they actually wanted to just restart from the last checkpoint.

 * **Flip items in your inventory using the keyboard** - Creates key bindings to flip (not just rotate) items when playing with a keyboard and mouse, just like you can do if you play with a controller.
    * Default bindings: 
		 * Flip up: Home
		 * Flip down: End
		 * Flip left: Insert
		 * Flip right: Page Up

 * **Rebind keyboard QTE keys** - The vanilla game doesn't allow the user to change the main QTE keys, being "X" and "C". Changing them via an .ini file is possible, but the on screen prompts don't change to match the new keys, leading to confusion. This mod changes the default QTE keys to be "A" and "D", both customizable and with proper on screen prompt support.

 * **Allow high resolution SFD files** - Allocate more memory for SFD movie files, and properly scale its resolution display above 512x336. Not tested beyond 1920x1080.

 * **RaiseVertexAlloc** - Allocate more memory for some vertex buffers. This prevents a crash that can happen when playing with a high FOV.
 
 * **RaiseInventoryAlloc** - Allocate more memory for the inventory screen, preventing crashes with high-poly models inside ss_pzzl.dat.

 * **UseMemcpy** - Makes the game use the memcpy function instead of MemorySwap, possibly resulting in some slight performance improvement.

### Installation

Extract dinput8.dll and dinput8.ini to "Steam\steamapps\common\Resident Evil 4\Bin32".

Options can be configured inside dinput8.ini.

Massive thanks to [emoose](https://github.com/emoose) for his incredible help with many of these features.