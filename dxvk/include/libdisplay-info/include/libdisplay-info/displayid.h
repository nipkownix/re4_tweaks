#ifndef DI_DISPLAYID_H
#define DI_DISPLAYID_H

/**
 * libdisplay-info's low-level API for VESA Display Identification Data
 * (DisplayID).
 *
 * The library implements DisplayID version 1.3, available at:
 * https://vesa.org/vesa-standards/
 */

#include <stdbool.h>
#include <stdint.h>

/**
 * DisplayID data structure.
 */
struct di_displayid;

/**
 * Get the DisplayID version.
 */
int
di_displayid_get_version(const struct di_displayid *displayid);

/**
 * Get the DisplayID revision.
 */
int
di_displayid_get_revision(const struct di_displayid *displayid);

/**
 * Product type identifier, defined in section 2.3.
 */
enum di_displayid_product_type {
	/* Extension section */
	DI_DISPLAYID_PRODUCT_TYPE_EXTENSION = 0x00,
	/* Test structure or equipment */
	DI_DISPLAYID_PRODUCT_TYPE_TEST = 0x01,
	/* Display panel or other transducer, LCD or PDP module, etc. */
	DI_DISPLAYID_PRODUCT_TYPE_DISPLAY_PANEL = 0x02,
	/* Standalone display device, desktop monitor, TV monitor, etc. */
	DI_DISPLAYID_PRODUCT_TYPE_STANDALONE_DISPLAY = 0x03,
	/* Television receiver */
	DI_DISPLAYID_PRODUCT_TYPE_TV_RECEIVER = 0x04,
	/* Repeater/translator */
	DI_DISPLAYID_PRODUCT_TYPE_REPEATER = 0x05,
	/* Direct drive monitor */
	DI_DISPLAYID_PRODUCT_TYPE_DIRECT_DRIVE = 0x06,
};

/**
 * Get the DisplayID product type.
 */
enum di_displayid_product_type
di_displayid_get_product_type(const struct di_displayid *displayid);

/**
 * DisplayID data block tag.
 */
enum di_displayid_data_block_tag {
	/*Product Identification Data Block */
	DI_DISPLAYID_DATA_BLOCK_PRODUCT_ID = 0x00,
	/* Display Parameters Data Block */
	DI_DISPLAYID_DATA_BLOCK_DISPLAY_PARAMS = 0x01,
	/* Color Characteristics Data Block */
	DI_DISPLAYID_DATA_BLOCK_COLOR_CHARACT = 0x02,
	/* Video Timing Modes Type I - Detailed Timings Data Block */
	DI_DISPLAYID_DATA_BLOCK_TYPE_I_TIMING = 0x03,
	/* Video Timing Modes Type II - Detailed Timings Data Block */
	DI_DISPLAYID_DATA_BLOCK_TYPE_II_TIMING = 0x04,
	/* Video Timing Modes Type III - Short Timings Data Block */
	DI_DISPLAYID_DATA_BLOCK_TYPE_III_TIMING = 0x05,
	/* Video Timing Modes Type IV - DMT Timings Data Block */
	DI_DISPLAYID_DATA_BLOCK_TYPE_IV_TIMING = 0x06,
	/* Supported Timing Modes - VESA DMT Timings Data Block */
	DI_DISPLAYID_DATA_BLOCK_VESA_TIMING = 0x07,
	/* Supported Timing Modes - CTA-861 Timings Data Block */
	DI_DISPLAYID_DATA_BLOCK_CEA_TIMING = 0x08,
	/* Video Timing Range Data Block */
	DI_DISPLAYID_DATA_BLOCK_TIMING_RANGE_LIMITS = 0x09,
	/* Product Serial Number Data Block */
	DI_DISPLAYID_DATA_BLOCK_PRODUCT_SERIAL = 0x0A,
	/* General-purpose ASCII String Data Block */
	DI_DISPLAYID_DATA_BLOCK_ASCII_STRING = 0x0B,
	/* Display Device Data Data Block */
	DI_DISPLAYID_DATA_BLOCK_DISPLAY_DEVICE_DATA = 0x0C,
	/* Interface Power Sequencing Data Block */
	DI_DISPLAYID_DATA_BLOCK_INTERFACE_POWER_SEQ = 0x0D,
	/* Transfer Characteristics Data Block */
	DI_DISPLAYID_DATA_BLOCK_TRANSFER_CHARACT = 0x0E,
	/* Display Interface Data Block */
	DI_DISPLAYID_DATA_BLOCK_DISPLAY_INTERFACE = 0x0F,
	/* Stereo Display Interface Data Block */
	DI_DISPLAYID_DATA_BLOCK_STEREO_DISPLAY_INTERFACE = 0x10,
	/* Video Timing Modes Type V - Short Timings Data Block */
	DI_DISPLAYID_DATA_BLOCK_TYPE_V_TIMING = 0x11,
	/* Tiled Display Topology Data Block */
	DI_DISPLAYID_DATA_BLOCK_TILED_DISPLAY_TOPO = 0x12,
	/* Video Timing Modes Type VI - Detailed Timings Data Block */
	DI_DISPLAYID_DATA_BLOCK_TYPE_VI_TIMING = 0x13,
};

/**
 * A DisplayID data block.
 */
struct di_displayid_data_block;

/**
 * Get a DisplayID data block tag.
 */
enum di_displayid_data_block_tag
di_displayid_data_block_get_tag(const struct di_displayid_data_block *data_block);

/**
 * Display parameters feature support flags, defined in section 4.2.3.
 */
struct di_displayid_display_params_features {
	/* Audio support on video interface */
	bool audio;
	/* Audio inputs are provided separately from the video interface */
	bool separate_audio_inputs;
	/* Audio information received via the video interface will automatically
	 * override any other audio input channels provided */
	bool audio_input_override;
	/* Display supports the VESA Display Power Management (DPM) standard */
	bool power_management;
	/* Display is capable of only a single fixed timing */
	bool fixed_timing;
	/* Display is capable of supporting timings at only a single fixed pixel
	 * format */
	bool fixed_pixel_format;
	/* Display supports ACP, ISRC1 or ISRC2 packets */
	bool ai;
	/* Display by default will de-interlace any interlaced video input */
	bool deinterlacing;
};

/**
 * Display parameters data block, defined in section 4.2.
 */
struct di_displayid_display_params {
	/* Image size in millimeters accurate to the thenths place, zero if unset */
	float horiz_image_mm, vert_image_mm;
	/* Native format size in pixels, zero if unset */
	int32_t horiz_pixels, vert_pixels;
	/* Feature flags */
	const struct di_displayid_display_params_features *features;
	/* Transfer characteristic gamma, zero if unset */
	float gamma;
	/* Aspect ratio (long axis divided by short axis) */
	float aspect_ratio;
	/* Color bit depth (dynamic range) */
	int32_t bits_per_color_overall, bits_per_color_native;
};

/**
 * Get display parameters from a DisplayID data block.
 *
 * Returns NULL if the data block tag isn't
 * DI_DISPLAYID_DATA_BLOCK_DISPLAY_PARAMS.
 */
const struct di_displayid_display_params *
di_displayid_data_block_get_display_params(const struct di_displayid_data_block *data_block);

enum di_displayid_type_i_timing_stereo_3d {
	/* This timing is always displayed monoscopic (no stereo) */
	DI_DISPLAYID_TYPE_I_TIMING_STEREO_3D_NEVER = 0x00,
	/* This timing is always displayed in stereo */
	DI_DISPLAYID_TYPE_I_TIMING_STEREO_3D_ALWAYS = 0x01,
	/* This timing is displayed in mono or stereo depending on a user action
	 * (wearing the stereo glasses, etc.) */
	DI_DISPLAYID_TYPE_I_TIMING_STEREO_3D_USER = 0x02,
};

enum di_displayid_type_i_timing_aspect_ratio {
	DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_1_1 = 0x00,
	DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_5_4 = 0x01,
	DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_4_3 = 0x02,
	DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_15_9 = 0x03,
	DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_16_9 = 0x04,
	DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_16_10 = 0x05,
	DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_64_27 = 0x06,
	DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_256_135 = 0x07,
	DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_UNDEFINED = 0x08,
};

enum di_displayid_type_i_timing_sync_polarity {
	DI_DISPLAYID_TYPE_I_TIMING_SYNC_NEGATIVE = 0x00,
	DI_DISPLAYID_TYPE_I_TIMING_SYNC_POSITIVE = 0x01,
};

/**
 * Type I timing, defined in section 4.4.1.
 */
struct di_displayid_type_i_timing {
	double pixel_clock_mhz; /* mega-hertz */
	bool preferred;
	enum di_displayid_type_i_timing_stereo_3d stereo_3d;
	bool interlaced;
	enum di_displayid_type_i_timing_aspect_ratio aspect_ratio;
	int32_t horiz_active, vert_active;
	int32_t horiz_blank, vert_blank;
	int32_t horiz_offset, vert_offset;
	int32_t horiz_sync_width, vert_sync_width;
	enum di_displayid_type_i_timing_sync_polarity horiz_sync_polarity, vert_sync_polarity;
};

/**
 * Get type I timings from a DisplayID data block.
 *
 * The returned array is NULL-terminated.
 *
 * Returns NULL if the data block tag isn't
 * DI_DISPLAYID_DATA_BLOCK_TYPE_I_TIMING.
 */
const struct di_displayid_type_i_timing *const *
di_displayid_data_block_get_type_i_timings(const struct di_displayid_data_block *data_block);

/**
 * Get DisplayID data blocks.
 *
 * The returned array is NULL-terminated.
 */
const struct di_displayid_data_block *const *
di_displayid_get_data_blocks(const struct di_displayid *displayid);

#endif
