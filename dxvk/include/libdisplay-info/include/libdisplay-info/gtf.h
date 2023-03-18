#ifndef DI_GTF_H
#define DI_GTF_H

#include <stdbool.h>

/**
 * Low-level API for Generalized Timing Formula Standard version 1.1.
 */

/**
 * Type of frequency parameter used in di_gtf_options.ip_freq_rqd.
 */
enum di_gtf_ip_param {
	/* Vertical frame frequency (Hz) */
	DI_GTF_IP_PARAM_V_FRAME_RATE,
	/* Horizontal frequency (kHz) */
	DI_GTF_IP_PARAM_H_FREQ,
	/* Pixel clock rate (MHz) */
	DI_GTF_IP_PARAM_H_PIXELS,
};

/**
 * Input options for GTF.
 */
struct di_gtf_options {
	/* Number of active image pixels displayed on a line, not including any
	 * margin */
	int h_pixels;
	/* Number of vertical lines in the displayed image */
	int v_lines;
	/* Whether margins are required */
	bool margins_rqd;
	/* Indicates which frequency parameter is specified in ip_freq_rqd */
	enum di_gtf_ip_param ip_param;
	/* Vertical frame frequency (in Hz), horizontal frequency (in kHz) or
	 * pixel clock rate (in MHz) */
	double ip_freq_rqd;
	/* Whether interlaced is required */
	bool int_rqd;
	/* Blanking formula gradient */
	double m;
	/* Blanking formula offset */
	double c;
	/* Blanking formula scaling factor */
	double k;
	/* Blanking formula scaling factor weighting */
	double j;
};

#define DI_GTF_DEFAULT_M 600.0
#define DI_GTF_DEFAULT_C 40.0
#define DI_GTF_DEFAULT_K 128.0
#define DI_GTF_DEFAULT_J 20.0

/**
 * Output timing data for GTF.
 */
struct di_gtf_timing {
	int h_pixels, v_lines;
	int h_sync, v_sync;
	int h_front_porch, h_back_porch;
	int v_front_porch, v_back_porch;
	int h_border, v_border;
	double pixel_freq_mhz; /* in mega-hertz */
};

/**
 * Compute a timing via the GTF formula.
 */
void di_gtf_compute(struct di_gtf_timing *t, const struct di_gtf_options *options);

#endif
