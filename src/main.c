#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "coredata/allocators/allocator.h"

typedef struct Slice {
    void* value;
    size_t size;
} Slice;

char* read_file(LinearAllocator* alloc, char path[64], size_t* size)
{ // {{{
    if (!size) { return NULL; }

    // Open the file
    FILE* file = fopen(path, "r");
    if (errno) {
        printf("Failed to open file '%s'. Error %s\n", path, strerror(errno));
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    printf("Opened file '%s' (%ld bytes)\n", path, file_size);
    fseek(file, 0, 0);
    *size = file_size;

    // Load file into the RAM
    char* file_ptr = linear_allocator_malloc(alloc, file_size + 1);
    file_ptr[file_size] = 0;
    size_t total_read = 0;
    size_t bytes_read = 0;
    while(bytes_read = fread(file_ptr + bytes_read, sizeof(char), file_size, file)) {
        printf("Reading %ld bytes into %p + %ld\n", bytes_read, file_ptr, total_read);
        total_read += bytes_read;

        if (feof(file)) { break; }
        if (ferror(file)) {
            printf("Failed to read file '%s'.\n", path);
            fclose(file);
            return NULL;
        }
    }

    // Cleanup
    assert(total_read == file_size);
    fclose(file);

    return file_ptr;
} // }}}


struct Token {
    enum TokenType
    { // {{{
      // Single-character tokens.
      TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
      TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
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
    Slice token_string;
};

// Parser helper function
// {{{
char* parser_peek(Slice src, size_t current_index)
{
    if (current_index + 1 > src.size) { return NULL; }
    return src.value + (current_index + 1);
}
char* parser_advance(Slice src, size_t *current_index)
{
    if ((*current_index) + 1 > src.size) { return NULL; }
    return src.value + (++(*current_index));
}
void parser_add_token(LinearAllocator* alloc, enum TokenType type, Slice* tokens)
{
    struct Token* tok = linear_allocator_malloc(alloc, sizeof(struct Token));
    tok->token = TOKEN_EOF;
    tokens->size++;
}
// }}}

Slice parse_str(LinearAllocator* alloc, Slice source)
{
    size_t begin = 0;
    size_t end = 0;

    if (alloc->_begin == NULL) {
        linear_allocator_realloc(alloc, 64);
    }
    Slice tokens = { alloc->_end, 0 };

    char* c = source.value;
    while(c) {
        if (begin >= source.size) {
            parser_add_token(alloc, TOKEN_EOF, &tokens);
            break;
        }
        switch (*c) {
            case '(': parser_add_token(alloc, TOKEN_LEFT_PAREN, &tokens);
            case ')': parser_add_token(alloc, TOKEN_RIGHT_PAREN, &tokens);
            case '{': parser_add_token(alloc, TOKEN_LEFT_BRACE, &tokens);
            case '}': parser_add_token(alloc, TOKEN_RIGHT_BRACE, &tokens);
            case ';': parser_add_token(alloc, TOKEN_SEMICOLON, &tokens);
            case ',': parser_add_token(alloc, TOKEN_COMMA, &tokens);
            case '.': parser_add_token(alloc, TOKEN_DOT, &tokens);
            case '-': parser_add_token(alloc, TOKEN_MINUS, &tokens);
            case '+': parser_add_token(alloc, TOKEN_PLUS, &tokens);
            case '/': parser_add_token(alloc, TOKEN_SLASH, &tokens);
            case '*': parser_add_token(alloc, TOKEN_STAR, &tokens);
        }
        c = parser_advance(source, &begin);
    }

    return tokens;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("Usage: %%s <file path>\n");
    }
    LinearAllocator* main_alloc = linear_allocator_new();

    size_t file_size;
    char* file_ptr = read_file(main_alloc, argv[1], &file_size);
    //char* file_ptr = "hello world\n";
    if (file_ptr) {
        Slice tokens = parse_str(main_alloc, (Slice){file_ptr, file_size});

        for (int i = 0; i < tokens.size; i++) {
            printf("%d\n", ((struct Token*)tokens.value)[i].token);
        }
    }

    printf("Used: %ld Allocated: %ld\n",
            linear_allocator_size(main_alloc),
            linear_allocator_cap(main_alloc));
    linear_allocator_free(main_alloc);
    return 0;
}
