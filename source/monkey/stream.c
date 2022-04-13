#include "monkey/stream.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

Stream* StreamFromFile(FILE* file) {
	Stream* result = calloc(1, sizeof(Stream));
	result->file = file;
	return result;
}

Stream* StreamFromText(char* text, size_t text_length) {
	Stream* result = calloc(1, sizeof(Stream));
	result->text = text;
	result->text_length = text_length;
	result->text_position = 0;
	return result;
}

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

int64_t ReadStream(Stream* stream, char* buffer, size_t buffer_size) {
	if (stream->file) {
		return (int64_t)fread(buffer, 1, buffer_size, stream->file);
	}
	if (stream->text_position == stream->text_length) {
		return -1;
	}
	size_t bytes_to_read = MIN(buffer_size, stream->text_length - stream->text_position);
	memcpy(buffer, stream->text + stream->text_position, bytes_to_read);
	stream->text_position += bytes_to_read;
	return (int64_t)bytes_to_read;
}

#define INITIAL_BUFFER_SIZE 256

int64_t ReadStreamLine(char** buffer, size_t* buffer_size, Stream* stream) {
	size_t bytes_read = 0;
	if (*buffer_size == 0) {
		*buffer_size = INITIAL_BUFFER_SIZE;
		*buffer = malloc(*buffer_size);
	}
	while (1) {
		int c;
		if (stream->file) {
			c = fgetc(stream->file);
		} else {
			if (stream->text_position == stream->text_length) {
				c = EOF;
			} else {
				c = (unsigned char)stream->text[stream->text_position++];
			}
		}
		if (c == EOF) {
			if (bytes_read == 0) {
				return -1;
			}
			break;
		}
		if (c == '\n') {
			break;
		}
		(*buffer)[bytes_read++] = (char)c;
		if (bytes_read == *buffer_size) {
			*buffer_size *= 2;
			*buffer = realloc(*buffer, *buffer_size);
		}
	}
	(*buffer)[bytes_read] = '\0';
	return (int64_t)bytes_read;
}

int64_t WriteStream(Stream* stream, const char* buffer, size_t buffer_size) {
	if (stream->file) {
		return (int64_t)fwrite(buffer, 1, buffer_size, stream->file);
	}
	size_t bytes_to_copy = MIN(buffer_size, stream->text_length - stream->text_position);
	if (bytes_to_copy == 0) {
		return -1;
	}
	memcpy(stream->text + stream->text_position, buffer, bytes_to_copy);
	stream->text_position += bytes_to_copy;
	return (int64_t)bytes_to_copy;
}

int64_t StreamPrintf(Stream* stream, const char* format, ...) {
	va_list args;
	va_start(args, format);
	int64_t result = 0;
	if (stream->file) {
		result = vfprintf(stream->file, format, args);
	} else {
		result = vsnprintf(stream->text + stream->text_position,
				stream->text_length - stream->text_position, format, args);
		if (result > 0) {
			stream->text_position += (size_t)result;
		}
	}
	va_end(args);
	return result;
}

void RewindStream(Stream* stream) {
	if (stream->file) {
		rewind(stream->file);
	} else {
		stream->text_position = 0;
	}
}

void CloseStream(Stream* stream) {
	if (stream->file) {
		(void)fclose(stream->file);
	}
	free(stream);
}
