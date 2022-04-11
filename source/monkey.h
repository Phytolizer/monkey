#pragma once

/**
 * @brief Simply initializes the name member to the name of the project
 */
typedef struct {
	const char* name;
} Monkey;

/**
 * @brief Creates an instance of library with the name of the project
 */
Monkey create_monkey(void);

/**
 * @brief Destroys resources held by the library
 */
void destroy_monkey(Monkey* lib);
