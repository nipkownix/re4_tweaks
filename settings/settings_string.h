#pragma once
const char* defaultSettings = R""""(
[DISPLAY]
; Additional FOV value. 20 seems good for most cases.
FOVAdditional = 0.0

; Fixes the incorrect aspect ratio when playing in ultrawide resolutions, 
; preventing the image from being cut off and the HUD appearing off-screen.
; Only tested in 21:9.
FixUltraWideAspectRatio = true

; Force V-Sync to be disabled. For some reason the vanilla game doesn't provide a functional way to do this.
DisableVsync = false

; Allows the game to use non-60Hz refresh rates in fullscreen, fixing the black screen issue people have
; when starting it.
FixDisplayMode = true

; Determines a custom refresh rate for the game to use.
; Requires FixDisplayMode to be enabled.
; -1 will make it try to use the current refresh rate as reported by Windows.
CustomRefreshRate = -1

; Restores a transparency effect that is missing from the item pickup screeen.
RestorePickupTransparency = true

; This filter was originally meant to add an extra glow effect on certain fire sources, but it was broken
; when the game was ported to the Xbox 360, making the entire image have an orange tint overlay applied to it.
; (if you're using the HD Project, you should disable this option)
DisableBrokenFilter03 = true

; Fixes a problem related to a vertex buffer that caused the image to be slightly blurred,
; making the image much sharper and clearer.
FixBlurryImage = true

; Disables the film grain overlay that is present in most sections of the game.
DisableFilmGrain = true

; Restores DoF blurring from the GC version, which was removed/unimplemented in later ports.
EnableGCBlur = true

; Restores outer-scope blurring when using a scope, which was removed/unimplemented in later ports.
EnableGCScopeBlur = true

; Whether to use a borderless-window when using windowed-mode.
WindowBorderless = false

; Position to draw the game window when using windowed mode.
; -1 will use the games default (usually places it at 0,0)
WindowPositionX = -1
WindowPositionY = -1

; Remember the last window position. This automatically updates the "WindowPositionX" and "WindowPositionY" values.
RememberWindowPos = false

[MOUSE]
; Makes it so the mouse turns the character instead of controlling the camera.
; "Modern" aiming mode in the game's settings is recomended.
UseMouseTurning = true

; Sensitivity for the mouse turning feature.
; Min 0.5, max 2.0.
TurnSensitivity = 1.0

; Makes the game use Raw Input for aiming and turning (if MouseTurning is enabled).
; Greatly improves mouse input by removing negative/positive accelerations that were being applied both by the game and by Direct Input.
; This option automatically enables the game's "Modern" aiming mode, and is incompatible with the "Classic" mode.
UseRawMouseInput = true

; When using the "Modern" mouse setting, the game locks the camera position to the aiming position, making both move together.
; Although this is the expected behavior in most games, some people might prefer to keep the original camera behavior while also having the benefits from "Modern" aiming.
; Enabling this will also restore the horizontal aiming sway that was lost when the devs implemented "Modern" aiming.
UnlockCameraFromAim = false

; Prevents the camera from being randomly displaced after you zoom with a sniper rifle when using keyboard and mouse.
FixSniperZoom = true

; When zooming in and out with the sniper rifle, this option makes the "focus" animation look similar to how it looks like with a controller.
; Requires EnableGCBlur to be enabled.
FixSniperFocus = true

; Prevents the game from overriding your selection in the "Retry/Load" screen when moving the mouse before confirming an action.
; This bug usually causes people to return to the main menu by mistake, when they actually wanted to just restart from the last checkpoint.
FixRetryLoadMouseSelector = true

[KEYBOARD]
; Key bindings for flipping items in the inventory screen when using keyboard and mouse.
; Normally, you can only rotate them with the keyboard, not flip them. Flipping was possible in the old PC port and is
; possible using a controller.
FlipItemUp    = HOME
FlipItemDown  = END
FlipItemLeft  = INSERT
FlipItemRight = PAGEUP

; Key bindings for QTE keys when playing with keyboard and mouse.
; Unlike the "official" way of rebinding keys through usr_input.ini, this
; option also changes the on-screen prompt to properly match the selected key.
QTE_key_1 = D
QTE_key_2 = A

; Changes sprint key to act like a toggle instead of needing to be held.
UseSprintToggle = false

; Game will turn keys invisible for certain unsupported keyboard languages
; Enabling this should make game use English keys for unsupported ones instead
; (if game supports your current language it should still use it however)
FallbackToEnglishKeyIcons = true

[CONTROLLER]
; Change the controller sensitivity. For some reason the vanilla game doesn't have an option to change it for controllers, only for the mouse.
; Min 0.5, max 4.0.
ControllerSensitivity = 1.0

; Removes unnecessary deadzones that were added for Xinput controllers.
RemoveExtraXinputDeadzone = true

; Change the Xinput controller deadzone.
; The game's default is 1, but that seems unnecessarily large, so we default to 0.4 instead.
; Min 0.0, max 3.5.
XinputDeadzone = 0.4

[FRAME RATE]
; Fixes the speed of falling items in 60 FPS, making them not fall at double speed.
FixFallingItemsSpeed = true

; Fixes speed of backwards turning when using framerates other than 30FPS.
FixTurningSpeed = true

; When running in 60 FPS, some QTEs require extremely fast button presses to work. This gets even worse in Professional difficulty,
; making it seem almost impossible to survive the minecart and the statue bridge QTEs.
; This fix makes QTEs that involve rapid button presses much more forgiving.
FixQTE = true

; Fixes difference between 30/60FPS on physics applied to Ashley.
FixAshleyBustPhysics = true

[MISC]
; Path to DLL to wrap, comment or leave empty to wrap system DLL
; (only set this if you need re4_tweaks to 'chain-load' another DLL that makes use of the same filename)
;WrappedDLLPath = 

; Allows overriding the costumes, making it possible to combine Normal/Special 1/Special 2 costumes.
OverrideCostumes = false

; Possible costume values:
; Leon: Jacket, Normal, Vest, RPD, Mafia
; Ashley: Normal, Popstar, Armor
; Ada: RE2, Spy, Normal
LeonCostume = Jacket
AshleyCostume = Normal
AdaCostume = Normal

; Unlocks the JP-only classic camera angles during Ashley's segment.
AshleyJPCameraAngles = false

; Allows overriding the level of violence in the game.
; Use -1 to leave as your game EXEs default, 0 for low-violence mode (as used in JP/GER version), or 2 for full violence.
ViolenceLevelOverride = -1

; Allows selling the (normally unused) handgun silencer to the merchant.
; If you use a mod that changes the merchant price tables you might want to disable this
; But if you're only using RE4HD you can leave this enabled
AllowSellingHandgunSilencer = true

; Allows the game to display Leon's mafia outfit ("Special 2") on cutscenes.
AllowMafiaLeonCutscenes = true

; Silence Ashley's armored outfit ("Special 2").
; For those who also hate the constant "Clank Clank Clank".
SilenceArmoredAshley = false

; Allows Ashley to Suplex enemies in very specific situations.
; (previously was only possible in the initial NTSC GameCube ver., was patched out in all later ports.)
AllowAshleySuplex = false

; Disables most of the QTEs, making them pass automatically.
DisableQTE = false

; Unlike the previous option, this only automates the "mashing" QTEs, making them pass automatically. Prompts are still shown!
AutomaticMashingQTE = false

; Whether to skip the Capcom etc intro logos when starting the game.
SkipIntroLogos = false

; Enables the "tool menu" debug menu, present inside the game but unused, and adds a few custom menu entries ("SAVE GAME", "DOF/BLUR MENU", etc).
; Can be opened with the LT+LS button combination (or CTRL+F3 by default on keyboard).
; If enabled on the 1.0.6 debug build it'll apply some fixes to the existing debug menu, fixing AREA JUMP etc, but won't add our custom entries due to lack of space.
; (may have a rare chance to cause a heap corruption crash when loading a save, but if the game loads fine then there shouldn't be any chance of crashing)
EnableDebugMenu = false

[MEMORY]
; Allocate more memory for SFD movie files, and properly scale its resolution display above 512x336.
; Not tested beyond 1920x1080.
AllowHighResolutionSFD = true

; Allocate more memory for some vertex buffers.
; This prevents a crash that can happen when playing with a high FOV.
RaiseVertexAlloc = true

; Allocate more memory for the inventory screen, preventing crashes with high-poly models inside ss_pzzl.dat.
RaiseInventoryAlloc = true

[HOTKEYS]
; Key combinations for various re4_tweaks features
; All keys can be combined (requiring multiple to be pressed at the same time) by using + symbol between key names
; (see top of Settings.cpp file for possible key names to use)

; Key combination to open the re4_tweaks config menu
ConfigMenu = F1

; Key combination to open the re4_tweaks debug console (only in certain re4_tweaks builds)
Console = F2

; Key combination to make the "tool menu" debug menu appear
; Requires EnableDebugMenu to be enabled.
DebugMenu = CTRL+F3

; MouseTurning camera modifier. When holding this key combination, MouseTurning is temporarily activated/deactivated.
MouseTurningModifier = ALT

; Key combo for jump tricks during Jet-ski sequence (normally RT+LT on controller)
JetSkiTricks = LMOUSE+RMOUSE

[WARNING]
; This version of RE4 only works properly if played at 30 or 60 FPS. Anything else can and will cause numerous amounts of
; different bugs, most of which aren't even documented. By default, re4_tweaks will warn you about these issues and change
; the FPS to either 30 or 60.
; If you have modified the game's config.ini file and changed the "variableframerate" option to something other than 30 or 60,
; please be aware of the potential issues before turning off this warning.
IgnoreFPSWarning = false

[IMGUI]
; Scale the font used in the configuration menu.
; Min 1.0, max 1.3.
FontSize = 1.000

; Disables the "Press key to open the configuration menu" tooltip.
DisableMenuTip = false

[LOG]
; Logs extra information.
VerboseLog = false
)"""";