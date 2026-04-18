#ifndef DI_DMT_H
#define DI_DMT_H

/**
 * libdisplay-info's low-level API for VESA Display Monitor Timing (DMT).
 *
 * The library implements VESA DMT version 1.0 revision 13.
 */

#include <stdint.h>

/**
 * A DMT timing.
 */
struct di_dmt_timing {
	/* DMT ID */
	uint8_t dmt_id;
	/* EDID standard timing 2-byte code, zero if unset */
	uint16_t edid_std_id;
	/* CVT 3-byte code, zero if unset */
	uint32_t cvt_id;
	/* Addressable pixels */
	int32_t horiz_video, vert_video;
	/* Field Refresh Rate in Hz */
	float refresh_rate_hz;
	/* Pixel clock in Hz */
	int32_t pixel_clock_hz;
	/* Horizontal/Vertical Blanking in pixels/lines */
	int32_t horiz_blank, vert_blank;
	/* Horizontal/Vertical Front Porch in pixels/lines */
	int32_t horiz_front_porch, vert_front_porch;
	/* Horizontal/Vertical Sync Pulse Width in pixels/lines */
	int32_t horiz_sync_pulse, vert_sync_pulse;
	/* Horizontal/Vertical Border in pixels/lines */
	int32_t horiz_border, vert_border;
};

#endif
