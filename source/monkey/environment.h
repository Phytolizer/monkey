#pragma once

#include "monkey/object.h"

#include <stdbool.h>

typedef struct Environment Environment;

/**
 * @brief Create an Environment for holding key-value pairs.
 *
 * @param outer the parent environment, or NULL for a root environment
 * @return Environment* the environment, or NULL if unsuccessful
 */
Environment* CreateEnvironment(Environment* outer);

/**
 * @brief Destroy a created Environment. Frees all internal data.
 *
 * @param env the environment
 */
void DestroyEnvironment(Environment* env);

/**
 * @brief Copy an Environment, including its data.
 *
 * @param env the environment
 * @return Environment* the copy
 */
Environment* CopyEnvironment(Environment* env);

/**
 * @brief Get a value from the Environment.
 *
 * @param env the environment
 * @param name the value's key
 * @return Object* the value, or NULL if not found
 */
Object* GetEnvironment(Environment* env, const char* name);

/**
 * @brief Put a value into the Environment. The Environment owns the provided data.
 *
 * @param env the environment
 * @param name the key to store the value under
 * @param val the value
 * @return bool whether there was already a value with this name
 */
bool PutEnvironment(Environment* env, char* name, Object* val);
