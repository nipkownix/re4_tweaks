#ifndef BITS_H
#define BITS_H

/**
 * Utility functions to operate on bits.
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Check whether a byte has a bit set.
 */
static inline bool
has_bit(uint8_t val, size_t index)
{
	return val & (1 << index);
}

/**
 * Extract a bit range from a byte.
 *
 * Both offsets are inclusive, start from zero, and high must be greater than low.
 */
static inline uint8_t
get_bit_range(uint8_t val, size_t high, size_t low)
{
	size_t n;
	uint8_t bitmask;

	assert(high <= 7 && high >= low);

	n = high - low + 1;
	bitmask = (uint8_t) ((1 << n) - 1);
	return (uint8_t) (val >> low) & bitmask;
}

#endif
