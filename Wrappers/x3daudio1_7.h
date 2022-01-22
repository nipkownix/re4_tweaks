#pragma once

#define VISIT_PROCS_X3DAUDIO1_7(visit) \
	visit(X3DAudioInitialize, jmpaddr) \
	visit(X3DAudioCalculate, jmpaddr) \
	visit(CreateFX, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(x3daudio1_7, dll, VISIT_PROCS_X3DAUDIO1_7, VISIT_PROCS_BLANK)
#endif
