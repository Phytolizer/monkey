#pragma once

#include <stddef.h>

#define SPAN_TYPE(T) \
	struct { \
		T* begin; \
		T* end; \
		size_t length; \
	}

#define SPAN_FROM_BOUNDS(begin, end) \
	{ (begin), (end), (end) - (begin) }

#define SPAN_WITH_LENGTH(begin, length) \
	{ (begin), (begin) + (length), (length) }

#define SPAN_EMPTY \
	{ NULL, NULL, 0 }
