#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @brief Stream is an abstraction for text I/O.
 *
 * Streams are used to read and write text data. They may be backed by either a file or a string.
 */
typedef struct {
	FILE* file;
	char* text;
	size_t text_length;
	size_t text_position;
} Stream;

/**
 * @brief StreamFromFile creates a stream from a file.
 *
 * @param file The file to read from.
 * @return The stream.
 */
Stream* StreamFromFile(FILE* file);

/**
 * @brief StreamFromText creates a stream from a string.
 *
 * @param text The string to read from.
 * @param text_length The length of the string.
 * @return The stream.
 */
Stream* StreamFromText(char* text, size_t text_length);

/**
 * @brief ReadStream reads characters into a buffer.
 *
 * @param stream The stream to read from.
 * @param buffer The buffer to read into.
 * @param buffer_size The size of the buffer.
 * @return The number of characters read, or -1 if end-of-stream was reached and no characters were
 * read.
 */
int64_t ReadStream(Stream* stream, char* buffer, size_t buffer_size);

/**
 * @brief ReadStreamLine reads a line of text from a stream. The buffer is reallocated to fit the
 * entire line.
 *
 * @param buffer The buffer to read into.
 * @param buffer_size The size of the buffer.
 * @param stream The stream to read from.
 * @return The number of characters read, or -1 if end-of-stream was reached and no characters were
 * read.
 */
int64_t ReadStreamLine(char** buffer, size_t* buffer_size, Stream* stream);

/**
 * @brief WriteStream writes characters from a buffer.
 *
 * @param stream The stream to write to.
 * @param buffer The buffer to write from.
 * @param buffer_size The size of the buffer.
 * @return The number of characters written, or -1 if an error occurred.
 */
int64_t WriteStream(Stream* stream, const char* buffer, size_t buffer_size);

/**
 * @brief StreamPrintf writes a formatted string to a stream.
 *
 * @param stream The stream to write to.
 * @param format The format string.
 * @return The number of characters written, or -1 if an error occurred.
 */
int64_t StreamPrintf(Stream* stream, const char* format, ...);

/**
 * @brief CloseStream closes the stream.
 *
 * @param stream The stream to close.
 */
void CloseStream(Stream* stream);
