#include "token.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

char *token_push_to_string(char *str, char c) {
  int len = 0;
  if (str != NULL) {
    len = strlen(str);
  }
  str = realloc(str, len + 2);

  assert(str != NULL);

  str[len] = c;
  str[len + 1] = '\0';

  return str;
}

int token_tokenize(char *str, char delimeter, char ***tokens) {

  // TODO: Reimplement the tokenize logic
}

char *token_get_next(char **tokens, int *token_size) {
  if (*token_size < 0)
    return NULL;

  if (tokens == NULL)
    return NULL;

  char *next_token = tokens[*token_size];

  if (*token_size < 1) {

    (*token_size)--;
    return *tokens;

    // token_size--;
    // free(tokens);
  }

  for (int i = 1; i < *token_size; i++) {
    tokens[i - 1] = tokens[i];
  }
  // memmove(tokens[*token_size], tokens[*token_size + 1],
  //         strlen(tokens[*token_size + 1]));

  (*token_size)--;
  return next_token;
}

int token_len(char *str) { return strlen(str); }
