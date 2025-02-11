#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <poll.h>
#include <fcntl.h>
#include <unistd.h>

#include <ctype.h>

#include <stdbool.h>

#include "coredata/allocators/allocator.h"

#define TOKEN_LIST \
  X(TOKEN_LEFT_PAREN) X(TOKEN_RIGHT_PAREN) \
  X(TOKEN_LEFT_BRACE) X(TOKEN_RIGHT_BRACE) \
  X(TOKEN_LEFT_SQUARE) X(TOKEN_RIGHT_SQUARE) \
  \
  X(TOKEN_COMMA) X(TOKEN_DOT) \
  X(TOKEN_MINUS) X(TOKEN_PLUS) \
  X(TOKEN_SEMICOLON) X(TOKEN_SLASH) \
  X(TOKEN_STAR) X(TOKEN_BANG) \
  X(TOKEN_BANG_EQUAL) X(TOKEN_EQUAL) \
  X(TOKEN_EQUAL_EQUAL) X(TOKEN_GREATER) \
  X(TOKEN_GREATER_EQUAL) X(TOKEN_LESS) \
  X(TOKEN_LESS_EQUAL) \
  \
  X(TOKEN_IDENTIFIER) X(TOKEN_STRING) X(TOKEN_NUMBER) \
  X(TOKEN_AND) X(TOKEN_CLASS) X(TOKEN_ELSE) \
  X(TOKEN_FALSE) X(TOKEN_FOR) X(TOKEN_FUN) \
  X(TOKEN_IF) X(TOKEN_NIL) X(TOKEN_OR) \
  X(TOKEN_PRINT) X(TOKEN_RETURN) X(TOKEN_SUPER) \
  X(TOKEN_THIS) X(TOKEN_TRUE) X(TOKEN_VAR) \
  X(TOKEN_WHILE) \
  \
  X(TOKEN_ERROR) X(TOKEN_EOF) \

#define X(name) #name,
char* TokenTypeString[] =
{ TOKEN_LIST };
#undef X
#define X(name) name,
enum TokenType { TOKEN_LIST };
#undef X

LinearSlice read_file(LinearAllocator* alloc, char path[64], size_t* size)
{ // {{{
  if (!size) { return (LinearSlice){NULL, 0, 0}; }

  // Open the file
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    printf("Failed to open file '%s'\n", path);
    return (LinearSlice){NULL, 0, 0};
  }


  struct pollfd fdp;
  fdp.fd = fd;
  fdp.events = POLLIN;


  printf("Opening %s\n", path);
  for(int i = 0; i < 10; i++) {
    if (poll(&fdp, 1, 10) > 0) {
      if ((fdp.revents & POLLIN) > 0) {
        break;
      } else {
        printf(".");
      }
    }
  }
  printf("\n");

  FILE* file = fdopen(fd, "r");
  if (!file) {
    printf("Failed to open file '%s'. Error %s\n", path, strerror(errno));
    return (LinearSlice){NULL, 0, 0};
  }

  // Get file size
  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  printf("File '%s' (%ld bytes)\n", path, file_size);
  fseek(file, 0, 0);
  *size = file_size;

  // Load file into the RAM
  if (alloc->_begin == NULL) {
    linear_allocator_realloc(alloc, file_size + 1);
  }
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

typedef struct Token {
  enum TokenType token; // }}}
  LinearSlice token_string;
  } Token;

// Parser helper function
// {{{
char parser_peek_next(LinearSlice src, size_t current_index)
{
  if (current_index + 1 > src.size) { return '\0'; }
  return *(char*)linear_allocator_at_slice(
      (LinearSlice){src.allocator, current_index, 1});
}
char parser_peek_prev(LinearSlice src, size_t current_index)
{
  if (current_index - 1 < 0) { return '\0'; }
  return *(char*)linear_allocator_at_slice(
      (LinearSlice){src.allocator, current_index - 1, 1});
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
  ((Token*)linear_allocator_at_slice(tok))->token = type;
  (*tokens)++;
  printf("%s\n", TokenTypeString[type]);
}
// }}}

LinearSlice parse_str(LinearAllocator* alloc, LinearSlice source)
{
  size_t position = 0;
  //size_t end = 0;

  if (alloc->_begin == NULL) {
    linear_allocator_realloc(alloc, 64);
  }
  size_t tokens_begin = alloc->_end - alloc->_begin;
  size_t tokens_count = 0;


  bool is_string = false;
  bool is_char = false;

  char c = ' ';
  for(int i = 0;; i++) {
    if (position >= source.size) {
      parser_add_token(alloc, TOKEN_EOF, &tokens_count);
      break;
    }
    switch (c) {
      case ' ':
      case '\r':
      case '\n':
      case '\t':
        c = parser_advance(source, &position);
        continue;
      case '\"':
        if (parser_peek_prev(source, position) != '\\') {
          is_string = !is_string;
        }
        break;
      case '\'':
        if (parser_peek_prev(source, position) != '\\') {
          is_char = !is_char;
        }
        break;

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
      case '!': parser_add_token(alloc, parser_peek_next(source, position) == '='
                    ? TOKEN_BANG_EQUAL : TOKEN_BANG , &tokens_count); break;
      case '=': parser_add_token(alloc, parser_peek_next(source, position) == '='
                    ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL , &tokens_count); break;
      case '<': parser_add_token(alloc, parser_peek_next(source, position) == '='
                    ? TOKEN_LESS_EQUAL : TOKEN_LESS , &tokens_count); break;
      case '>': parser_add_token(alloc, parser_peek_next(source, position) == '='
                    ? TOKEN_GREATER_EQUAL : TOKEN_GREATER , &tokens_count); break;
      case '/':
        {
          if (is_string || is_char) break;
          if (parser_peek_next(source, position) == '/') {
            c = parser_advance(source, &position);
            printf("\e[32m//");
            while ((c = parser_advance(source, &position))) {
              if (c == '\n') {
                break;
              }
              printf("%c", c);
            }
            printf("\e[0m\n");
          } else if (parser_peek_next(source, position) == '*') {
            c = parser_advance(source, &position);
            printf("\e[32m/*");
            while ((c = parser_advance(source, &position))) {
              if (c == '*' && parser_peek_next(source, position) == '/') {
                break;
              }
              printf("%c", c);
            }
            printf("*/\e[0m\n");
          }
        } break;
      default:
        break;
    }
    static bool am_string = false;
    if (is_string || is_char) {
      if (am_string) {
        printf("\e[33m%c\e[0m", parser_peek_prev(source, position));
      }
      am_string = true;
    } else {
      if (am_string) {
        printf("\n");
        am_string = false;
      }
    }

    c = parser_advance(source, &position);
  }
  printf("\n");

  //for (int i = 0; i < tokens_count; i++) {
  //  Token* tok = linear_allocator_at_slice(
  //      (LinearSlice){alloc, tokens_begin + (i * sizeof(Token)), sizeof(Token)}
  //      );
  //  printf("T: %s\n", TokenTypeString[tok->token]);
  //}
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
  (void)file_slice;

  if (file_slice.size) {
    parse_str(main_alloc, file_slice);
  }

  printf("Used: %ld Allocated: %ld\n",
      linear_allocator_size(main_alloc),
      linear_allocator_cap(main_alloc));
  linear_allocator_free(main_alloc);

  return 0;
}
