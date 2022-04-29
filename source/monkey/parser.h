#pragma once

#include "monkey/ast.h"
#include "monkey/lexer.h"
#include "monkey/string.h"

typedef struct Parser Parser;

/**
 * @brief CreateParser creates a new parser.
 * @param lexer The lexer to use.
 * @return A new parser.
 */
Parser* CreateParser(Lexer* lexer);

/**
 * @brief ParseProgram parses the program.
 * @param parser The parser to use.
 * @return The program.
 */
Program* ParseProgram(Parser* parser);

/**
 * @brief DestroyParser destroys a parser.
 * @param parser The parser to destroy.
 */
void DestroyParser(Parser* parser);

MonkeyStringVector ParserErrors(Parser* parser);
