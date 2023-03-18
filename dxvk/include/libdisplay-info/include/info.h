#ifndef INFO_H
#define INFO_H

/**
 * Private header for the high-level API.
 */

#include <libdisplay-info/info.h>

struct di_info {
	struct di_edid *edid;

	char *failure_msg;
};

#endif
