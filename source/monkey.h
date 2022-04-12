#pragma once

/**
 * @brief MonkeyTokenState is an opaque struct that holds the state of the
 * token module.
 */
#include "monkey/macros.h"
typedef struct MonkeyTokenState MonkeyTokenState;

/**
 * @brief Monkey is a struct that holds the public state of the Monkey library.
 */
typedef struct {
	const char* name;
} Monkey;

/**
 * @brief CreateMonkey creates an instance of the library and returns it.
 */
Monkey* CreateMonkey(void);

/**
 * @private
 */
MONKEY_INTERNAL MonkeyTokenState* MonkeyGetTokenState(Monkey* monkey);

/**
 * @brief Destroys resources held by the library
 */
void DestroyMonkey(Monkey* lib);
