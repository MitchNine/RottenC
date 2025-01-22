#include <stdint.h>
#include <stdio.h>

#define TOKEN_MAP_SINGLE_SIZE 18
#define TOKEN_MAP_SINGLE \
    X(TOKEN_L_BRACE, "{") \
    X(TOKEN_R_BRACE, "}") \
    X(TOKEN_L_SQUARE, "[") \
    X(TOKEN_R_SQUARE, "}") \
    X(TOKEN_L_BRACKET, "(") \
    X(TOKEN_R_BRACKET, ")") \
    X(TOKEN_L_TRIANGLE, "<") \
    X(TOKEN_R_TRIANGLE, ">") \
    X(TOKEN_DOUBLE_QUOTE, "\"") \
    X(TOKEN_QUOTE, "\'") \
    X(TOKEN_DOT, ".") \
    X(TOKEN_COMMA, ",") \
    X(TOKEN_F_SLASH, "/") \
    X(TOKEN_B_SLASH, "\\") \
    X(TOKEN_EQUAL, "=") \
    X(TOKEN_PLUS, "+") \
    X(TOKEN_MINUS, "-") \
    X(TOKEN_MULT, "*") \


// Generate token enum
#define X(name, symbol) name,
enum TOKENS_SINGLE { TOKEN_MAP_SINGLE };
#undef X

// Generate token symbols
#define X(name, symbol) symbol,
char* tokens_single_symbols[] = { TOKEN_MAP_SINGLE };
#undef X


int main()
{
    for (int i = 0; i < 2; i++)
        printf("%s\n", tokens_single_symbols[i]);
    return 0;
}
