
# re4_tweaks

### Description
A project designed to fix and tweak the "UHD" port of Resident Evil 4 on [Steam](https://store.steampowered.com/app/254700/Resident_Evil_4/).

### Current features

 * **Increase FOV** - Increases the default FOV. Useful to prevent motion sickness in some people.

 * **Fix ultrawide aspect ratio** - Fixes the incorrect aspect ratio when playing in ultrawide resolutions, preventing the image from being cut off and the HUD appearing off-screen. Only tested in 21:9.

 * **Fix sniper zoom** - When playing with keyboard and mouse, this fixes an annoying bug which would cause your camera to be off-center after you zoomed in with a sniper rifle.

* **Fix QTE** - When running in 60 FPS, some QTEs require extremely fast button presses to work. This gets even worse in Professional difficulty, making it seem almost impossible to survive the minecart and the statue bridge QTEs. This fix makes QTEs that involve rapid button presses much more forgiving.

 * **Restore pickup screen transparency** - Restores a transparency effect that was present on the GameCube and Wii versions of RE4, but has been missing since the game was ported to Xbox 360. 
	* Comparison: [Before](https://i.imgur.com/AdP9AtK.jpg) | [After](https://i.imgur.com/BSKjq6b.jpg)

* **Disable post processing** - Disables post processing effects such as motion blur and the "color filter" option. Most notably, it disables the game's forced FXAA, making the image much sharper and clearer. 
The anti-aliasing option found in the settings menu is unaffected.

 * **Disable film grain** - Disables the film grain overlay that is present in most sections of the game.

 * **Flip items in your inventory using the keyboard** - Creates key bindings to flip (not just rotate) items when playing with a keyboard and mouse, just like you can do if you play with a controller.
    * Default bindings: 
		 * Flip up: Home
		 * Flip down: End
		 * Flip left: Insert
		 * Flip right: Page Up

 * **Rebind keyboard QTE keys** - The vanilla game doesn't allow the user to change the main QTE keys, being "X" and "C". Changing them via an .ini file is possible, but the on screen prompts don't change to match the new keys, leading to confusion. This mod changes the default QTE keys to be "A" and "D", both customizable and with proper on screen prompt support.

### Installation

Extract winmm.dll and winmm.ini to "Steam\steamapps\common\Resident Evil 4\Bin32".

Options can be configured inside winmm.ini.
