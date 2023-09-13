// We use the `vdprintf` function, which is a GNU Extension, and 
// were described in POSIX.1-2008, so we define the POSIX C source
// to avoid `gcc` yelling about implicit functions
#define _POSIX_C_SOURCE 200809L


#include <asm-generic/ioctls.h>
#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <stddef.h>

#define CTRLQ 17
#define CTRLS 19
#define BACKSPACE 127

void dbg(char *s, ...){
  va_list v;
  va_start(v, s);
  vdprintf(STDOUT_FILENO, s, v);
  va_end(v);
}

enum Key {
  // In decimal, NOT Octal
  CTRL_Q = 17,
  CTRL_S = 19,
  CTRL_C = 3,
  TAB = 9,
  ESC,
};

// Describes one row of the Editor

typedef struct row {
  int idx;         // Current index of the row
  int size;        // Number of chars, punctuations and tabs in this current row
  int render_size; // size after adding all the tabs
  char *content;   // content of the row
  char *render;    // content ready for render - with tabs expanded
} row;

// Editor config & state

struct Editor{
  int x;
  int y;
  row *r;
  int numrow;
  int coloff;
  int rowoff;
  int screenrow;
  int screencol;
  bool save;
  char *file_name;
};

static struct Editor E;



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

int get_cursor_position(int *row, int *col) {
  int i = 0;
  char buf[8];
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, buf + i, 1) != 1)
      return -1;
    if (buf[i] == 'R')
      break;
    i++;
  }
  if (buf[0] != ESC || buf[1] != 27)
    return -1;
  if (sscanf(buf + 2, "%d;%d", row, col) != 2)
    return -1;
  // For debug purposes
  return 0;
}

int init_editor(char *file) {
  // struct termios orig;

  // Allocates Memory for the original_term termios struct. Fails if the malloc
  // fails
  term.original_term = (struct termios *)malloc(sizeof(struct termios));
  if (term.original_term == NULL) {
    perror("Failed to initialize original stuct");
    return -1;
  }
  tcgetattr(term.fd, term.original_term);
  if (term.original_term == NULL) {
    printf("Failed Initialization\n");
    return -1;
  }
  // yes. DRY code for you.
  E.x = E.y = E.numrow = E.coloff = E.rowoff = E.screenrow = E.screencol = 0;
  E.save = false;
  E.file_name = NULL;
  E.r = NULL;
  E.file_name = file;
  return 0;
}

// STATUS: complete

void disable_raw_mode() {
  if (term.original_term == NULL)
    printf("NULLED POINTER\n");
  if (term.is_raw)
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, term.original_term) == -1)
      perror("Failed to disable raw mode");
  free(term.original_term);
  free(E.r);
  // Clear the alternate screen
  write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7);
  // Switch back to normal screen -- NOT Specifically VT100, but should work on most modern 
  // terminals and emulators. If the terminal is not capable of such feature, just discard
  // the escape sequence.
  write(STDOUT_FILENO, "\x1b[?1049l", strlen("\x1b[?1049l"));

  // `\x1b[1;32m` is just escape sequence for printing bold, green colored text
  printf("\n\x1b[1;32mExited Successfully from Ronto!\x1b[0m\n");
  term.is_raw = 0;
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


// STATUS: Incomplete
void get_window_size(int *row, int *col) {
  struct winsize w;
  if (ioctl(term.fd, TIOCGWINSZ, &w) == -1) {
    // Do SOMETHING
    // Handle when ioctl fails
  }

  /*
  FOR DEBUG
  TODO: Clean Up after testing
  */
  *row = w.ws_row;
  *col = w.ws_col;
}

// TODO: Implement save_file functionality
void save_file(void) { return; }

// A buffer formatter. It appends messages to be sent to the std output, and
// flushes them all at once.
void bf(void) { return; }

int add_row(int pos, char *buf, ssize_t len) {
  E.r = realloc(E.r, sizeof(row) * (E.numrow + 1));
  // "Error" Handeling
  assert(E.r != NULL);

  E.r[pos].idx = pos;

  // TODO: May have to revise this logic.
  // Is it pos > E.numrow or pos >= E.numrow
  if (pos < E.numrow) {
    // dbg("Run");
    memmove(E.r + pos + 1, E.r + pos,
            sizeof(E.r[0]) * (E.numrow - pos)); // Implicit object creation
    for (int i = pos + 1; i < E.numrow; i++)
      E.r[i].idx++;
  }
  E.r[pos].size = len;
  E.r[pos].content = malloc(len + 1); // + 1 for '\0'
  memcpy(E.r[pos].content, buf, len);
  E.r[pos].render = NULL;
  E.r[pos].render_size = 0;
  E.numrow++;
  // TODO: Update the Row now. i.e. expand raw characters to render field
  return 0;
}

void add_char_at(char c, int at, int rowpos){
  if (!E.r[rowpos].content){
    E.r[rowpos].content = malloc(1);
    assert(E.r[rowpos].content!=NULL);
  }
  // if (sizeof(E.r[rowpos].content)>at+1){
  //   E.r[rowpos].size = sizeof(E.r[rowpos].content) + 1;
  // }
  // 1+ Hours of BUG.
  E.r[rowpos].content = realloc(E.r[rowpos].content, E.r[rowpos].size+1);
  E.r[rowpos].content[at] = c;
  // Add null char at the end
  E.r[rowpos].size++;
}

void insert_key(char c) {
  int rp = E.rowoff + E.y;
  int cp = E.coloff + E.x;
  if (rp >= E.numrow) {
    // TODO: implement this function
    add_row(rp, "", 0);
  }
  add_char_at(c, cp, rp);
  E.x++;
  // if (E.y == E.r[rp].size - 1) {
  //   E.x++;
  //   E.y = 0;
  // } else {
  //   E.y++;
  // }
}

void delete_at(int rpos, int at){
  // E.r[rpos].content[at] = '\0';
  E.r[rpos].size--;
  E.x--;
}

// TODO: Fix bug
void delete(){
  // No Op if there is no character to remove
  if (E.x<1) return;
  int row = E.rowoff + E.y;
  int at = E.coloff + E.x;
  delete_at(row, at);
}

// Carriage Return + Newline
char *qmessage = "\r\nPressed Control + Q, so quitting\r\n";
char *smessage = "\r\nPressed Control + S, so saving\r\n";

// Maybe some editor config or other parameter
int key_up(void) {
  char c;
  // char seq[3];
  int num;
  while ((num = read(term.fd, &c, 1)) == 0)
    ;
  assert(num != -1);
  switch (c) {
    // DO Something
    case ESC:
    // Handle Escape Sequence
    return 0;
    break;
  default:
    // dbg("Pressed: %d", c);
    return c;
    break;
  }
}

// FEAT: rudimentary functionality complete
void handle_key_press() {
  int c = key_up();
  switch (c) {
  case CTRL_Q:
    write(STDOUT_FILENO, qmessage, strlen(qmessage));
    exit(0);
    break;

  case CTRL_S:
    write(STDOUT_FILENO, smessage, strlen(smessage));
    save_file();
  case CTRL_C:
    // TODO: Implement redumentary clipboard support. At least,
    // copy-the-whole-file feature
    break;
  case ESC:
    // TODO: some vim-like key-bindings? For future updates
    break;
  case BACKSPACE:
    delete();
    break;
  default:
    insert_key(c);
    // Handle default case i.e add it to the character buffer
    break;
  }
}

// TODO: Expand tabs
void expand_rows(void) {
  for (int i = 0; i < E.numrow; i++) {
    if (E.r[i].content==NULL) return;
    E.r[i].render = malloc(E.r[i].size + 1);
    assert(E.r[i].render != NULL);
    memcpy(E.r[i].render, E.r[i].content, E.r[i].size);
    write(STDOUT_FILENO, E.r[i].render, E.r[i].size);
    free(E.r[i].render);
  }
}

void refresh_screen(void) {
  // As we move cursor to home each time during refresh, we just erase from current line i.e. the first, to the last line
  // TODO: Modify for view buffers
  char *write_buf = "\x1b[H\x1b[J";
  write(STDOUT_FILENO, write_buf, strlen(write_buf));
  expand_rows();
  return;
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
  char *file_name = NULL;
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
      return -1;

    case '?':
      fprintf(stderr,
              "Unrecognized Option. Use --help to display allowed options");
      return -1;
    }
  }
  if (file) {
    if (isalnum_str(file_name) != 0)
      file_name = argv[2];
    printf("%s\n", file_name);
  }
  if (flush)
    // Goto alternate screen buffer
    write(STDOUT_FILENO,"\x1b[?1049h" ,strlen("\x1b[?1049h"));

  atexit(disable_raw_mode);

  init_editor(file_name);
  enable_raw_mode();
  while (1) {
    handle_key_press();
    refresh_screen();
  }
  return 0;
}
