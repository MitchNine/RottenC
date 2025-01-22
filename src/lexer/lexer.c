#include "lexer.h"

struct Hashmap* create_tokens()
{
    struct Hashmap* tokens = hashmap_create(256);

#define DEFINE_TOKEN(name, token)\
    hashmap_set(tokens, name, token, sizeof(#token));

    DEFINE_TOKEN("LBRACE", "{");
    DEFINE_TOKEN("RBRACE", "}");
    DEFINE_TOKEN("LBRACKET", "(");
    DEFINE_TOKEN("RBRACKET", ")");
    DEFINE_TOKEN("LSQUARE", "[");
    DEFINE_TOKEN("RSQUARE", "]");

#undef DEFINE_TOKEN

    return tokens;
}
