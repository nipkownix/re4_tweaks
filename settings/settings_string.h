#pragma once
const char* defaultSettings = R""""([DISPLAY]
; Additional FOV value. 20 seems good for most cases.
FOVAdditional = 0.0

; Fixes the incorrect aspect ratio when playing in ultrawide resolutions, 
; preventing the image from being cut off and the HUD appearing off-screen.
; Only tested in 21:9.
FixUltraWideAspectRatio = true

; Makes it so that the vsync option found in the game's config.ini actually works.
FixVsyncToggle = true

; Restores transparency on the item pickup screeen.
RestorePickupTransparency = true

; This filter was originally meant to add an extra glow effect on certain fire sources, but it was broken
; when the game was ported to the Xbox 360, making the entire image have an orange tint overlay applied to it.
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

[MISC]
; Fixes animation inconsistencies between 30 and 60 FPS that were not addressed by the developers.
60fpsFixes = true

; When running in 60 FPS, some QTEs require extremely fast button presses to work. This gets even worse in Professional difficulty,
; making it seem almost impossible to survive the minecart and the statue bridge QTEs.
; This fix makes QTEs that involve rapid button presses much more forgiving.
FixQTE = true

; Whether to skip the Capcom etc intro logos when starting the game.
SkipIntroLogos = false

; Enables the "tool menu" debug menu, present inside the game but unused, and adds a few custom menu entries ("SAVE GAME", "DOF/BLUR MENU", etc).
; Can be opened with the LT+LS button combination (or CTRL+F3 on keyboard).
; If enabled on the 1.0.6 debug build it'll apply some fixes to the existing debug menu, fixing AREA JUMP etc, but won't add our custom entries due to lack of space.
; (may have a rare chance to cause a heap corruption crash when loading a save, but if the game loads fine then there shouldn't be any chance of crashing)
EnableDebugMenu = false

; Keyboard key-combination to make the "tool menu" debug menu appear
; All keys can be combined (requiring multiple to be pressed at the same time) by using + symbol between key names
; (see top of Settings.cpp file for possible key names to use)
DebugMenuKeyCombination = CTRL+F3

[MOUSE]
; Makes it so the mouse turns the character instead of controlling the camera.
; "Modern" aiming mode in the game's settings is recomended.
UseMouseTurning = true
TurnSensitivity = 1.0

; Prevents the camera from being randomly displaced after you zoom with a sniper rifle when using keyboard and mouse.
FixSniperZoom = true

; Prevents the game from overriding your selection in the "Retry/Load" screen when moving the mouse before confirming an action.
; This bug usually causes people to return to the main menu by mistake, when they actually wanted to just restart from the last checkpoint.
FixRetryLoadMouseSelector = true

[KEYBOARD]
; Key bindings for flipping items in the inventory screen when using keyboard and mouse.
; Normally, you can only rotate them with the keyboard, not flip them. Flipping was possible in the old PC port and is
; possible using a controller.
flip_item_up    = HOME
flip_item_down  = END
flip_item_left  = INSERT
flip_item_right = PAGEUP

; Key bindings for QTE keys when playing with keyboard and mouse.
; Unlike the "official" way of rebinding keys through usr_input.ini, this
; option also changes the on-screen prompt to properly match the selected key.
QTE_key_1 = D
QTE_key_2 = A

[MOVIE]
; Allocate more memory for SFD movie files, and properly scale its resolution display above 512x336.
; Not tested beyond 1920x1080.
AllowHighResolutionSFD = true

[MEMORY]
; Allocate more memory for some vertex buffers.
; This prevents a crash that can happen when playing with a high FOV.
RaiseVertexAlloc = true

; Allocate more memory for the inventory screen, preventing crashes with high-poly models inside ss_pzzl.dat.
RaiseInventoryAlloc = true

; Makes the game use the memcpy function instead of MemorySwap,
; possibly resulting in some slight performance improvement.
UseMemcpy = true

[FRAME RATE]
; This version of RE4 only works properly if played at 30 or 60 FPS. Anything else can and will cause numerous amounts of
; different bugs, most of which aren't even documented. By default, re4_tweaks will warn you about these issues and change
; the FPS to either 30 or 60.
; If you have modified the game's config.ini file and changed the "variableframerate" option to something other than 30 or 60,
; please be aware of the potential issues before turning off this warning.
IgnoreFPSWarning = false

[IMGUI]
; Scale the font used in the configuration menu.
; Min 1.0, max 1.3.
FontSize = 1.000)"""";