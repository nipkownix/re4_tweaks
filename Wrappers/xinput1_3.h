#pragma once

#define VISIT_PROCS_XINPUT1_3(visit) \
	visit(XInputGetState, jmpaddr) \
	visit(XInputSetState, jmpaddr) \
	visit(XInputGetCapabilities, jmpaddr) \
	visit(XInputEnable, jmpaddr) \
	visit(XInputGetDSoundAudioDeviceGuids, jmpaddr) \
	visit(XInputGetBatteryInformation, jmpaddr) \
	visit(XInputGetKeystroke, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(xinput1_3, dll, VISIT_PROCS_XINPUT1_3, VISIT_PROCS_BLANK)
#endif
