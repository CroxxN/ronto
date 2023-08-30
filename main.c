#include <asm-generic/ioctls.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>
#include<sys/ioctl.h>

// Basic Terminal Structure that holds some frequently used values - #GLOBAL
struct Terminal {
  /*
  The struct pointer definition is completely useless(especially because it's 
  global variable anyway). 
  One could easily use a non-pointer version. I just wanted to
  see & learn, so used a pointer version.
  Pitfalls here I come.
  */
  struct termios *original_term;

  bool is_raw;
  int fd;
} term = {NULL, 0, STDIN_FILENO};

// Checks whether a string is wholly alphanumeric or punctuation-ic subset

// Fails when the `string` is binary or non-alphanumeric & non-punctuation(._-)

int isalnum_str(char *string) {
  for (int i = 0; i < strlen(string); i++) {
    if (isalnum(string[i]) < 0 || string[i] == '.' || string[i] == '_' ||
        string[i] == '-')
      return -1;
  }
  return 0;
}

int init_editor() {
  // struct termios orig;

  // Allocates Memory for the original_term termios struct. Fails if the malloc fails
  term.original_term = (struct termios*) malloc(sizeof(struct termios));
  if (term.original_term == NULL){
    perror("Failed to initialize original stuct");
    return -1;
  }
  tcgetattr(term.fd, term.original_term);
  if (term.original_term == NULL){
    printf("Failed Initialization\n");
    return -1;
  }
  // term.original_term = orig;

  // For now No case is handeled
  return 0;
}

// Done

int enable_raw_mode() {
  struct termios raw;
  tcgetattr(term.fd, &raw);

  // Local Flag

  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

  // Input Flags

  raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON);

  // Output Flags

  raw.c_oflag &= ~(OPOST);

  // Control Flags

  raw.c_cflag |= (CS8); // No & mask, no bit flipping

  term.is_raw = 1;
  if (tcsetattr(term.fd, TCSAFLUSH, &raw) == -1) 
    perror("Failed to set terminal to raw mode");

  return 0;
}

// Done

void disable_raw_mode() {
  if (term.original_term == NULL) printf("NULLED POINTER\n");
  if (term.is_raw)
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, term.original_term) == -1) 
      perror("Failed to disable raw mode");
  term.is_raw = 0;
}


void get_window_size(int *row,int *col){
  struct winsize w;
  if (ioctl(term.fd, TIOCGWINSZ, &w) == -1){
    // Do SOMETHING
    // Handle when ioctl fails
  }

  /*
  FOR DEBUG
  TODO: Clean Up after testing
  */
  *row = w.ws_row;
  *col = w.ws_col;
  printf("%d:%d\n", *row, *col);
}



// Maybe some editor config or other parameter
void key_up(void){
  char c, seq[3];
  int num;
  while ((num = read(term.fd, &c, 1))==0);
  if (num==-1){
    perror("Failed to process keystroke");
    return;
  }
  while (1){
    switch (c) {
      // DO Something
      break;
    }
  }
    
}

// Maybe some editor config or other parameter
void handle_key_press(void){
  key_up();
  // DO Something
}



// Program Driver

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
  if (flush)
    // `\x1b[2J`& `\x1b[H` are different VT100 escape sequences.
  // `\x1b[2J` is for clearing the sreen
  // & `\x1b[H` puts the cursor at the top-left.
    write(STDIN_FILENO, "\x1b[2J\x1b[H", 7);


  atexit(disable_raw_mode);
  init_editor();
  enable_raw_mode();
  char input_char;
  int row, col;
  get_window_size(&row, &col);
  while (read(STDIN_FILENO, &input_char, 1) == 1 && input_char != 'q') {
    if (iscntrl(c)) {
      printf("%d\r\n", input_char);
    } else {

      printf("%c (%d)\r\n", input_char, input_char);
    }
  }
  return 0;
}
