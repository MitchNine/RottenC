#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "coredata/allocators/allocator.h"


LinearSlice read_file(LinearAllocator* alloc, char path[64], size_t* size)
{ // {{{
    if (!size) { return (LinearSlice){NULL, 0, 0}; }

    // Open the file
    FILE* file = fopen(path, "r");
    if (errno) {
        printf("Failed to open file '%s'. Error %s\n", path, strerror(errno));
        return (LinearSlice){NULL, 0, 0};
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    printf("Opened file '%s' (%ld bytes)\n", path, file_size);
    fseek(file, 0, 0);
    *size = file_size;

    // Load file into the RAM
    LinearSlice slice = linear_allocator_malloc(alloc, file_size + 1);
    char* file_ptr = linear_allocator_at_slice(slice);
    file_ptr[file_size] = 0;
    size_t total_read = 0;
    size_t bytes_read = 0;
    while((bytes_read = fread(file_ptr + bytes_read, sizeof(char), file_size, file))) {
        printf("Reading %ld bytes into %p + %ld\n", bytes_read, file_ptr, total_read);
        total_read += bytes_read;

        if (feof(file)) { break; }
        if (ferror(file)) {
            printf("Failed to read file '%s'.\n", path);
            fclose(file);
            return (LinearSlice){NULL, 0, 0};
        }
    }

    // Cleanup
    assert(total_read == file_size);
    fclose(file);

    return slice;
} // }}}

char* TokenTypeString[] =
{
    "TOKEN_LEFT_PAREN", "TOKEN_RIGHT_PAREN",
    "TOKEN_LEFT_BRACE", "TOKEN_RIGHT_BRACE",
    "TOKEN_LEFT_SQUARE", "TOKEN_RIGHT_SQUARE",
    "TOKEN_COMMA", "TOKEN_DOT", "TOKEN_MINUS",
    "TOKEN_PLUS", "TOKEN_SEMICOLON", "TOKEN_SLASH",

    "TOKEN_STAR", "TOKEN_BANG",
    "TOKEN_BANG_EQUAL", "TOKEN_EQUAL",
    "TOKEN_EQUAL_EQUAL", "TOKEN_GREATER",
    "TOKEN_GREATER_EQUAL", "TOKEN_LESS",
    "TOKEN_LESS_EQUAL",

    "TOKEN_IDENTIFIER", "TOKEN_STRING", "TOKEN_NUMBER",

    "TOKEN_AND", "TOKEN_CLASS", "TOKEN_ELSE",
    "TOKEN_FALSE", "TOKEN_FOR", "TOKEN_FUN",
    "TOKEN_IF", "TOKEN_NIL", "TOKEN_OR",
    "TOKEN_PRINT", "TOKEN_RETURN", "TOKEN_SUPER",
    "TOKEN_THIS", "TOKEN_TRUE", "TOKEN_VAR",
    "TOKEN_WHILE", "TOKEN_ERROR",

    "TOKEN_EOF",
};

typedef struct Token {
    enum TokenType
    { // {{{
      // Single-character tokens.
      TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
      TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
      TOKEN_LEFT_SQUARE, TOKEN_RIGHT_SQUARE,
      TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
      TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
      // One or two character tokens.
      TOKEN_BANG, TOKEN_BANG_EQUAL,
      TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
      TOKEN_GREATER, TOKEN_GREATER_EQUAL,
      TOKEN_LESS, TOKEN_LESS_EQUAL,
      // Literals.
      TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
      // Keywords.
      TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
      TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
      TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
      TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

      TOKEN_ERROR, TOKEN_EOF
    } token; // }}}
    LinearSlice token_string;
} Token;

// Parser helper function
// {{{
char parser_peek(LinearSlice src, size_t current_index)
{
    if (current_index + 1 > src.size) { return '\0'; }
    return *(char*)linear_allocator_at_slice(
            (LinearSlice){src.allocator, current_index, 1});
}
char parser_advance(LinearSlice src, size_t *current_index)
{
    if ((*current_index) + 1 > src.size) { return '\0'; }
    return *(char*)linear_allocator_at_slice(
            (LinearSlice){src.allocator, (*current_index)++, 1});
}
void parser_add_token(LinearAllocator* alloc, enum TokenType type, size_t* tokens)
{
    LinearSlice tok = linear_allocator_malloc(alloc, sizeof(Token));
    if (tok.size == 0) printf("ERR\n");
    ((Token*)linear_allocator_at_slice(tok))->token = type;
    (*tokens)++;
}
// }}}

LinearSlice parse_str(LinearAllocator* alloc, LinearSlice source)
{
    size_t begin = 0;
    size_t end = 0;

    if (alloc->_begin == NULL) {
        linear_allocator_realloc(alloc, 64);
    }
    size_t tokens_begin = alloc->_end - alloc->_begin;
    size_t tokens_count = 0;


    char c = ' ';
    for(int i = 0; i < 1024; i++) {
        if (begin >= source.size) {
            parser_add_token(alloc, TOKEN_EOF, &tokens_count);
            break;
        }
        switch (c) {
            case ' ':
            case '\r':
            case '\n':
            case '\t':
                c = parser_advance(source, &begin);
                continue;
            case '(': parser_add_token(alloc, TOKEN_LEFT_PAREN, &tokens_count); break;
            case ')': parser_add_token(alloc, TOKEN_RIGHT_PAREN, &tokens_count); break;
            case '{': parser_add_token(alloc, TOKEN_LEFT_BRACE, &tokens_count); break;
            case '}': parser_add_token(alloc, TOKEN_RIGHT_BRACE, &tokens_count); break;
            case '[': parser_add_token(alloc, TOKEN_LEFT_SQUARE, &tokens_count); break;
            case ']': parser_add_token(alloc, TOKEN_RIGHT_SQUARE, &tokens_count); break;
            case ';': parser_add_token(alloc, TOKEN_SEMICOLON, &tokens_count); break;
            case ',': parser_add_token(alloc, TOKEN_COMMA, &tokens_count); break;
            case '.': parser_add_token(alloc, TOKEN_DOT, &tokens_count); break;
            case '-': parser_add_token(alloc, TOKEN_MINUS, &tokens_count); break;
            case '+': parser_add_token(alloc, TOKEN_PLUS, &tokens_count); break;
            case '*': parser_add_token(alloc, TOKEN_STAR, &tokens_count); break;
            case '!': parser_add_token(alloc, 
                      parser_peek(source, begin) == '=' ? TOKEN_BANG_EQUAL : TOKEN_BANG
                      , &tokens_count); break;
            case '=': parser_add_token(alloc, 
                      parser_peek(source, begin) == '=' ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL
                      , &tokens_count); break;
            case '<': parser_add_token(alloc, 
                      parser_peek(source, begin) == '=' ? TOKEN_LESS_EQUAL : TOKEN_LESS
                      , &tokens_count); break;
            case '>': parser_add_token(alloc, 
                      parser_peek(source, begin) == '=' ? TOKEN_GREATER_EQUAL : TOKEN_GREATER
                      , &tokens_count); break;
            case '/': {
                      if (parser_peek(source, begin) == '/') {
                          c = parser_advance(source, &begin);
                          printf("\e[32m//");
                          while ((c = parser_advance(source, &begin))) {
                              if (c == '\n') break;
                              printf("%c", c);
                          }
                          printf("\e[0m\n");
                      }
                  } break;


        }
        c = parser_advance(source, &begin);
    }

    for (int i = 0; i < tokens_count; i++) {
        Token* tok = linear_allocator_at_slice(
            (LinearSlice){alloc, tokens_begin + (i * sizeof(Token)), sizeof(Token)}
        );
        printf("T: %s\n", TokenTypeString[tok->token]);
    }
    return (LinearSlice){ alloc, 0, 0};
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("Usage: %%s <file path>\n");
    }
    LinearAllocator* main_alloc = linear_allocator_new();

    size_t file_size;
    LinearSlice file_slice = read_file(main_alloc, argv[1], &file_size);
    if (file_slice.size) {
        parse_str(main_alloc, file_slice);
    }

    printf("Used: %ld Allocated: %ld\n",
            linear_allocator_size(main_alloc),
            linear_allocator_cap(main_alloc));
    linear_allocator_free(main_alloc);
    return 0;
}
