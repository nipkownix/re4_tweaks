#ifndef DI_EDID_DECODE_H
#define DI_EDID_DECODE_H

#include <stdbool.h>

struct uncommon_features {
	bool color_point_descriptor;
	bool color_management_data;
	bool cta_transfer_characteristics;
};

extern struct uncommon_features uncommon_features;

struct di_edid;
struct di_edid_detailed_timing_def;
struct di_edid_cta;
struct di_displayid;

void
print_edid(const struct di_edid *edid);

void
print_detailed_timing_def(const struct di_edid_detailed_timing_def *def);

void
print_cta(const struct di_edid_cta *cta);

void
print_displayid(const struct di_displayid *displayid);

#endif
