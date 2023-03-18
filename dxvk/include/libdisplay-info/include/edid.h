#ifndef EDID_H
#define EDID_H

/**
 * Private header for the low-level EDID API.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <libdisplay-info/edid.h>

#include "cta.h"
#include "displayid.h"

/**
 * The maximum number of EDID blocks (including the base block), defined in
 * section 2.2.1.
 */
#define EDID_MAX_BLOCK_COUNT 256
/**
 * The maximum number of EDID standard timings, defined in section 3.9.
 */
#define EDID_MAX_STANDARD_TIMING_COUNT 8
/**
 * The number of EDID byte descriptors, defined in section 3.10.
 */
#define EDID_BYTE_DESCRIPTOR_COUNT 4
/**
 * The size of an EDID byte descriptor, defined in section 3.10.
 */
#define EDID_BYTE_DESCRIPTOR_SIZE 18
/**
 * The maximum number of standard timings in an EDID display descriptor, defined
 * in section 3.10.3.6.
 */
#define EDID_MAX_DESCRIPTOR_STANDARD_TIMING_COUNT 6
/**
 * The maximum number of color points in an EDID color point descriptor, defined
 * in section 3.10.3.5.
 */
#define EDID_MAX_DESCRIPTOR_COLOR_POINT_COUNT 2
/**
 * The maximum number of established timings III in an EDID display descriptor,
 * defined in section 3.10.3.9.
 */
#define EDID_MAX_DESCRIPTOR_ESTABLISHED_TIMING_III_COUNT 44

struct di_edid_detailed_timing_def_priv {
	struct di_edid_detailed_timing_def base;
	struct di_edid_detailed_timing_analog_composite analog_composite;
	struct di_edid_detailed_timing_bipolar_analog_composite bipolar_analog_composite;
	struct di_edid_detailed_timing_digital_composite digital_composite;
	struct di_edid_detailed_timing_digital_separate digital_separate;
};

struct di_edid {
	struct di_edid_vendor_product vendor_product;
	int version, revision;

	bool is_digital;
	struct di_edid_video_input_analog video_input_analog;
	struct di_edid_video_input_digital video_input_digital;
	struct di_edid_screen_size screen_size;
	float gamma;
	struct di_edid_dpms dpms;
	enum di_edid_display_color_type display_color_type;
	struct di_edid_color_encoding_formats color_encoding_formats;
	struct di_edid_misc_features misc_features;

	struct di_edid_chromaticity_coords chromaticity_coords;
	struct di_edid_established_timings_i_ii established_timings_i_ii;

	/* NULL-terminated */
	struct di_edid_standard_timing *standard_timings[EDID_MAX_STANDARD_TIMING_COUNT + 1];
	size_t standard_timings_len;

	/* NULL-terminated */
	struct di_edid_detailed_timing_def_priv *detailed_timing_defs[EDID_BYTE_DESCRIPTOR_COUNT + 1];
	size_t detailed_timing_defs_len;

	/* NULL-terminated */
	struct di_edid_display_descriptor *display_descriptors[EDID_BYTE_DESCRIPTOR_COUNT + 1];
	size_t display_descriptors_len;

	/* NULL-terminated, doesn't include the base block */
	struct di_edid_ext *exts[EDID_MAX_BLOCK_COUNT];
	size_t exts_len;

	struct di_logger *logger;
};

struct di_edid_display_range_limits_priv {
	struct di_edid_display_range_limits base;
	struct di_edid_display_range_limits_secondary_gtf secondary_gtf;
	struct di_edid_display_range_limits_cvt cvt;
};

struct di_edid_display_descriptor {
	enum di_edid_display_descriptor_tag tag;
	/* Used for PRODUCT_SERIAL, DATA_STRING and PRODUCT_NAME,
	 * zero-terminated */
	char str[14];
	/* Used for RANGE_LIMITS */
	struct di_edid_display_range_limits_priv range_limits;
	/* Used for STD_TIMING_IDS, NULL-terminated */
	struct di_edid_standard_timing *standard_timings[EDID_MAX_DESCRIPTOR_STANDARD_TIMING_COUNT + 1];
	size_t standard_timings_len;
	/* Used for COLOR_POINT, NULL-terminated */
	struct di_edid_color_point *color_points[EDID_MAX_DESCRIPTOR_COLOR_POINT_COUNT + 1];
	size_t color_points_len;
	/* Used for ESTABLISHED_TIMINGS_III, NULL-terminated */
	const struct di_dmt_timing *established_timings_iii[EDID_MAX_DESCRIPTOR_ESTABLISHED_TIMING_III_COUNT + 1];
	size_t established_timings_iii_len;
	/* Used for DCM_DATA */
	struct di_edid_color_management_data dcm_data;
};

struct di_edid_ext {
	enum di_edid_ext_tag tag;
	/* Used for DI_EDID_EXT_CEA */
	struct di_edid_cta cta;
	/* Used for DI_EDID_EXT_DISPLAYID */
	struct di_displayid displayid;
};

/**
 * Create an EDID data structure.
 *
 * Callers do not need to keep the provided data pointer valid after calling
 * this function. Callers should destroy the returned pointer via
 * di_edid_destroy().
 */
struct di_edid *
_di_edid_parse(const void *data, size_t size, FILE *failure_msg_file);

/**
 * Destroy an EDID data structure.
 */
void
_di_edid_destroy(struct di_edid *edid);

/**
 * Parse an EDID detailed timing definition.
 */
struct di_edid_detailed_timing_def_priv *
_di_edid_parse_detailed_timing_def(const uint8_t data[static_array EDID_BYTE_DESCRIPTOR_SIZE]);

#endif
