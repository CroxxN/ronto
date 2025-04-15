#ifndef TOKEN_H_
#define TOKEN_H_

char *token_push_to_string(char *str, char c);
int token_tokenize(char *, char, char ***);
char *token_get_next(char **, int *);
int token_len(char *);

#endif
