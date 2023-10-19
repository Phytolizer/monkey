#include "monkey/stream.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
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
	result->textLength = text_length;
	result->textPosition = 0;
	return result;
}

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

int64_t ReadStream(Stream* stream, char* buffer, size_t buffer_size) {
	if (stream->file) {
		return (int64_t)fread(buffer, 1, buffer_size, stream->file);
	}
	if (stream->textPosition == stream->textLength) {
		return -1;
	}
	size_t bytesToRead = MIN(buffer_size, stream->textLength - stream->textPosition);
	memcpy(buffer, stream->text + stream->textPosition, bytesToRead);
	stream->textPosition += bytesToRead;
	return (int64_t)bytesToRead;
}

#define INITIAL_BUFFER_SIZE 256

int64_t ReadStreamLine(char** buffer, size_t* buffer_size, Stream* stream) {
	size_t bytesRead = 0;
	if (*buffer_size == 0) {
		*buffer_size = INITIAL_BUFFER_SIZE;
		*buffer = malloc(*buffer_size);
	}
	while (1) {
		int c;
		if (stream->file) {
			c = fgetc(stream->file);
		} else {
			if (stream->textPosition == stream->textLength) {
				c = EOF;
			} else {
				c = (unsigned char)stream->text[stream->textPosition++];
			}
		}
		if (c == EOF) {
			if (bytesRead == 0) {
				return -1;
			}
			break;
		}
		if (c == '\n') {
			break;
		}
		(*buffer)[bytesRead++] = (char)c;
		if (bytesRead == *buffer_size) {
			*buffer_size *= 2;
			*buffer = realloc(*buffer, *buffer_size);
		}
	}
	(*buffer)[bytesRead] = '\0';
	return (int64_t)bytesRead;
}

int64_t WriteStream(Stream* stream, const char* buffer, size_t buffer_size) {
	if (stream->file) {
		return (int64_t)fwrite(buffer, 1, buffer_size, stream->file);
	}
	size_t bytesToCopy = MIN(buffer_size, stream->textLength - stream->textPosition);
	if (bytesToCopy == 0) {
		return -1;
	}
	memcpy(stream->text + stream->textPosition, buffer, bytesToCopy);
	stream->textPosition += bytesToCopy;
	return (int64_t)bytesToCopy;
}

int64_t StreamPrintf(Stream* stream, const char* format, ...) {
	va_list args;
	va_start(args, format);
	int64_t result = 0;
	if (stream->file) {
		result = vfprintf(stream->file, format, args);
	} else {
		result = vsnprintf(stream->text + stream->textPosition,
				stream->textLength - stream->textPosition, format, args);
		if (result > 0) {
			stream->textPosition += (size_t)result;
		}
	}
	va_end(args);
	return result;
}

void RewindStream(Stream* stream) {
	if (stream->file) {
		(void)fseek(stream->file, 0, SEEK_SET);
	} else {
		stream->textPosition = 0;
	}
}

void CloseStream(Stream* stream) {
	if (stream->file) {
		(void)fclose(stream->file);
	}
	free(stream);
}
