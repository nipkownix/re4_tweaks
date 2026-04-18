#ifndef MEMORY_STREAM_H
#define MEMORY_STREAM_H

/**
 * Private header for the high-level API.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef _WIN32
#define MEMSTREAM_MAX_PATH 260
#endif

struct memory_stream {
	FILE *fp;
	char *str;
	size_t str_len;

#ifdef _WIN32
	char temp[MEMSTREAM_MAX_PATH];
#endif
};

bool
memory_stream_open(struct memory_stream *m);

char *
memory_stream_close(struct memory_stream *m);

#endif
