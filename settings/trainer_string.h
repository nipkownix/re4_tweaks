#pragma once
const char* defaultSettingsTrainer = R""""(
[TRAINER]
; Enables usage of the "Trainer" tab
Enable = False

; Allows overriding player speed value.
; (value can be set via the PlayerSpeedOverride value below)
EnablePlayerSpeedOverride = False

; Value to override player speed with
; Only takes effect if EnablePlayerSpeedOverride is enabled
PlayerSpeedOverride = 1.0

; Allows moving character via numpad when using noclip / "Disable Player Collision" patch
UseNumpadMovement = True

; Allows using mouse-wheel to move character up/down when using noclip / "Disable Player Collision" patch
UseMouseWheelUpDown = True

[TRAINER_HOTKEYS]
; Key combination to set mouse focus on trainer related UIs (EmManager/Globals)
FocusUI = F5

; Key combination to toggle the "Disable Player Collision" / no-clip patch.
NoclipToggle = 

; Key combination to toggle the "Player Speed Override" patch
SpeedOverrideToggle = 

; Key combination to move Ashley to the player's position
MoveAshleyToPlayer = 
)"""";
