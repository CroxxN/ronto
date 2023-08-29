#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

struct Terminal {
  struct termios original_term;
  bool is_raw;
  int fd;
  bool is_flush;
} term = {{},
0,
  STDIN_FILENO,
  0
};

// Checks whether a string is wholly alphanumeric

// Fails when the `string` is binary or non-alphanumeric

int isalnum_str(char *string) {
  for (int i = 0; i < strlen(string); i++) {
    if (isalnum(string[i]) < 0)
      return -1;
  }
  return 0;
}

int init_editor() {
  struct termios orig;
  tcgetattr(term.fd, &orig);
  term.original_term = orig;

  // For now No case is handeled
  return 0;
}

int enable_raw_mode() {
  struct termios raw;
  tcgetattr(term.fd, &raw);

  raw.c_lflag &= ~(ECHO | ICANON);
  term.is_raw =1;
  tcsetattr(term.fd, TCSAFLUSH,&raw);
  // Work in Progress

  return 0;
}

void disable_raw_mode() {
  if (term.is_raw)
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term.original_term);
  term.is_raw = 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("No Arguments Supplied");
    return -1;
  };
  char c;
  bool file = 0;
  bool flush = 0;
  char *file_name;
  extern char *optarg;
  extern int optind;
  while ((c = getopt(argc, argv, "nf:")) != -1) {
    switch (c) {
    case 'n':
      flush = 1;
      break;

    case 'f':
      file = 1;
      file_name = optarg;
      break;
    case ':':
      fprintf(stderr, "File Name required with the -f Parameter");
      break;

    case '?':
      fprintf(stderr,
              "Unrecognized Option. Use --help to display allowed options");
      break;
    }
  }
  if (file) {
    if (isalnum_str(file_name) != 0)
      file_name = argv[2];
    printf("%s\n", file_name);
  }
  term.is_flush = flush;
  atexit(disable_raw_mode);
  init_editor();
  enable_raw_mode();
  char input_char;
  while(read(STDIN_FILENO, &input_char, 1)==1 && input_char != 'q'){
    printf("%c (%d)\n", input_char, input_char);
  }
  return 0;
}
