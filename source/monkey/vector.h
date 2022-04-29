#pragma once

#define VECTOR_T(T) \
	struct { \
		T* data; \
		size_t size; \
		size_t capacity; \
	}

#define VECTOR_INIT \
	{ NULL, 0, 0 }

#define VECTOR_PUSH(V, Val) \
	do { \
		if ((V)->size == (V)->capacity) { \
			(V)->capacity = (V)->capacity == 0 ? 1 : (V)->capacity * 2; \
			(V)->data = realloc((V)->data, (V)->capacity * sizeof(*(V)->data)); \
		} \
		(V)->data[(V)->size++] = Val; \
	} while (0)

#define VECTOR_FREE(V) \
	do { \
		free((V)->data); \
		(V)->data = NULL; \
		(V)->size = 0; \
		(V)->capacity = 0; \
	} while (0)
