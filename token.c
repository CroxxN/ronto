#include "token.h"
#include "ronto.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// typedef struct {
//   int len;
//   char **inner;
// } Token;

int str_range(char *string, char delimeter) {
  if (string == NULL)
    return -1;

  int index = 0;

  if (string[0] == delimeter)

    for (int i = 0; i < strlen(string); i++)
      if (string[i] == delimeter)
        index++;
      else
        break;

  else

    for (int i = 0; i < strlen(string); i++)
      if (string[i] != delimeter)
        index++;
      else
        break;

  return index;
}

// int token_tokenize(char *str, char delimeter, char ***tokens) {}
// TODO: Reimplement the tokenize logic

Token *token_tokenize(char *str, char delimeter) {
  if (str == NULL)
    return NULL;

  Token *t = malloc(sizeof(Token));

  t->len = 0;
  t->curr = 0;
  t->inner = NULL;

  char *holder = str;
  int original_strlen = strlen(holder);

  int token_nums = 0;

  for (int i = 0; i < original_strlen; i++) {

    int range = str_range(holder + i, delimeter);

    t->len++;

    t->inner = realloc(t->inner, t->len * sizeof(char *));

    *(t->inner + token_nums) = malloc(range + 1);

    memcpy(*(t->inner + token_nums), holder + i, range);

    *(*(t->inner + token_nums) + range) = '\0';

    // MAYBE: i += (range -1)
    // NEXT_STEP:
    i += (range - 1);
    token_nums++;
  }

  return t;
}

char *token_get_next(Token *t) {
  if (t == NULL) {
    editor_log("[ERROR]: t == NULL\n");
    return NULL;
  }

  if (t->curr == t->len) {
    return NULL;
  }

  char *next = t->inner[t->curr];
  t->curr++;

  return next;
}

int token_len(char *str) { return strlen(str); }
