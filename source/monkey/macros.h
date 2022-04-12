#pragma once

/**
 * @brief MONKEY_INTERNAL is a macro that is used to mark functions as internal
 * and should not be used outside of the library.
 */
#define MONKEY_INTERNAL

/**
 * @brief MONKEY_FILE_LOCAL is a macro that is used to mark variables as file
 * local, i.e. cannot be used outside of the file.
 */
#define MONKEY_FILE_LOCAL static
