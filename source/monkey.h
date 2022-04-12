#pragma once

/**
 * @brief MonkeyTokenState is an opaque struct that holds the state of the
 * token module.
 */
typedef struct MonkeyTokenState MonkeyTokenState;

/**
 * @brief Simply initializes the name member to the name of the project
 */
typedef struct {
	const char* name;
	MonkeyTokenState* token;
} Monkey;

/**
 * @brief Creates an instance of library with the name of the project
 */
Monkey CreateMonkey(void);

/**
 * @brief Destroys resources held by the library
 */
void DestroyMonkey(Monkey* lib);
