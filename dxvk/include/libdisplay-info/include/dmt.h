#ifndef DMT_H
#define DMT_H

/**
 * Private header for VESA Display Monitor Timing.
 */

#include <sys/types.h>

#include <libdisplay-info/dmt.h>

extern const struct di_dmt_timing _di_dmt_timings[];
extern const size_t _di_dmt_timings_len;

#endif
