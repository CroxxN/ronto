#include "token.h"
#include "ronto.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// Get a range of continous sequence of
// EITHER delimeters OR non-delimeter characters
// Based on whether the first character of `string`
// is a delimeter or not, this function returns a `int`
// which is a sequence range of the number of delimeter or
// non-delimeter characters
int str_range(char *string, char delimeter) {
  if (string == NULL)
    return -1;

  int index = 0;

  // if the first character in the string is a delimeter
  if (string[0] == delimeter)

    for (unsigned long i = 0; i < strlen(string); i++)
      if (string[i] == delimeter)
        index++;
      else
        break;

  // if the first character in the string is NOT a delimeter
  else

    for (unsigned long i = 0; i < strlen(string); i++)
      if (string[i] != delimeter)
        index++;
      else
        break;

  return index;
}

// Tokenize `str` into a group of tokens. The characters are
// tokenized based on the criteria of whether or not they are
// delimeters
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

  // iterate over the characters in the string
  for (int i = 0; i < original_strlen; i++) {

    // get the range of the current character type
    int range = str_range(holder + i, delimeter);

    t->len++;

    t->inner = realloc(t->inner, t->len * sizeof(char *));

    *(t->inner + token_nums) = malloc(range + 1);

    // insert it into the token container
    memcpy(*(t->inner + token_nums), holder + i, range);

    // equivalent to E->inner[token_nums][range]
    *(*(t->inner + token_nums) + range) = '\0';

    // increase the current iterator to match the range
    i += (range - 1);

    // indicate that we got another token
    token_nums++;
  }

  return t;
}

// get the first token from a collection of tokens
char *token_get_next(Token *t) {
  if (t == NULL) {
    editor_log("[ERROR]: t == NULL\n");
    return NULL;
  }

  // when all the tokens are consumed
  if (t->curr == t->len) {
    return NULL;
  }

  // extract the current token from the index
  char *next = t->inner[t->curr];

  // advance the token pointer
  t->curr++;

  return next;
}

int token_len(char *str) { return strlen(str); }
