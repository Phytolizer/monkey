#pragma once

#include "span.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define BUFFER_TYPE(T) \
	struct { \
		T* data; \
		size_t length; \
		size_t capacity; \
	}

#define BUFFER_INIT \
	{ NULL, 0, 0 }

#define BUFFER_FREE(buffer) free(buffer.data)

#define BUFFER_EXPAND(buffer) \
	do { \
		if ((buffer)->length == (buffer)->capacity) { \
			(buffer)->capacity = (buffer)->capacity * 2 + 1; \
			(buffer)->data = \
					realloc((buffer)->data, (buffer)->capacity * sizeof((buffer)->data[0])); \
		} \
	} while (false)

#define BUFFER_PUSH(buffer, value) \
	do { \
		BUFFER_EXPAND(buffer); \
		(buffer)->data[(buffer)->length++] = (value); \
	} while (false)

#define BUFFER_AS_SPAN(buffer) SPAN_WITH_LENGTH((buffer).data, (buffer).length)
