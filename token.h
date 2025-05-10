#ifndef TOKEN_H_
#define TOKEN_H_

typedef struct {
  int len;
  int curr;
  char **inner;
} Token;

Token *token_tokenize(char *, char);
char *token_get_next(Token *);
int token_len(char *);

#endif
