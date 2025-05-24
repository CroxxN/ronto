
// TODO: Clean up these reduntant includes
#include "ronto.h"
#include "token.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// Must be a char array
static char FILE_TEMPLATE[] = "Untitled-XXXXXX";

// TODO: move this to `ronto.h`
// syntax highlight buffer

char SYNTAX_WORD[6];

// Enable or disable logging
// Disabled by default
int LOGGING = 0;

// For printing values to the stdout file descriptor
// DON'T USE -- See editor_log()
void dbg(char *s, ...) {
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
  BACKSPACE = 127,
  ENTER = 13,
  ESC = 27,
  // Not actual code, just a way to represent the key compactly
  ARROW_UP = 690,
  ARROW_DOWN,
  ARROW_LEFT,
  ARROW_RIGHT
};

static struct Editor E;

static struct buf b = {NULL, 0};

static struct Terminal term = {NULL, 0, STDIN_FILENO};

void editor_log(char *s, ...) {
  if (LOGGING == 0)
    return;
  va_list v;
  va_start(v, s);
  vfprintf(E.log, s, v);
}

// Checks whether a string is wholly alphanumeric or punctuation-ic subset

// Fails when the `string` is binary or non-alphanumeric & non-punctuation(._-)

int isalnum_str(char *string) {
  for (unsigned long i = 0; i < strlen(string); i++)
    if (iscntrl(string[i]))
      return -1;

  return 0;
}

// TODO:
int tabreplace(char *target, int pos) {
  (void)target;
  (void)pos;
  return 0;
}

// extend string
char *extend_string(char *dest, char *source) {
  if (source == NULL)
    return NULL;

  if (dest == NULL) {
    dest = malloc(strlen(source) + 1);
    dest[0] = '\0';
  } else
    dest = realloc(dest, strlen(dest) + strlen(source) + 1);

  dest = strcat(dest, source);
  return dest;
}

int get_cursor_position(int *row, int *col) {
  char buf[8];
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;

  read(STDIN_FILENO, buf, 6);
  editor_log(buf, "\n");
  if (buf[0] != ESC || buf[1] != 27)
    return -1;
  if (sscanf(buf + 2, "%d;%d", row, col) != 2)
    return -1;
  // For debug purposes
  return 0;
}

bool init_editor(char *file) {

  // Allocates Memory for the original_term termios struct. Fails if the
  // malloc fails
  term.original_term = (struct termios *)malloc(sizeof(struct termios));
  if (term.original_term == NULL) {
    perror("Failed to initialize original stuct");
    return -1;
  }
  tcgetattr(term.fd, term.original_term);
  if (term.original_term == NULL) {
    printf("Failed Initialization\n");
    exit(-1);
  }
  // verify that t is not NULL and `tmpfile()` call succeded
  E.temp_file = NULL;

  // yes. DRY code for you.
  E.x = E.y = E.numrow = E.coloff = E.rowoff = E.screenrow = E.screencol = 0;
  E.tabsize = 4;
  E.save = false;
  E.r = NULL;
  E.file = NULL;
  E.mode = NORMAL;

  if (LOGGING != 0)
    E.log = fopen("log", "a");

  get_window_size(&E.screenrow, &E.screencol);

  if (fopen(file, "rb") == NULL) {
    E.file = fopen(file, "w");
    return false;
  }
  bootstrap_file(file);
  E.file_name = file;
  return true;
}

// STATUS: complete

void disable_raw_mode(void) {
  if (term.original_term == NULL) {
    printf("NULLED POINTER\n");
    return;
  }
  if (term.is_raw)
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, term.original_term) == -1) {
      perror("Failed to disable raw mode");
      return;
    }

  term.is_raw = 0;
  free(term.original_term);
  free(E.r);
  if (E.file)
    fclose(E.file);
  // Clear the alternate screen
  bf("\x1b[H\x1b[2J");

  // Switch back to normal screen -- NOT Specifically VT100, but should work
  // on most modern terminals and emulators. If the terminal is not capable of
  // such feature, it just discards the escape sequence.
  bf("\x1b[?1049l");
  bf("\x1b[7h");
  bf_flush();
  // `\x1b[1;32m` is just escape sequence for printing bold, green colored
  // text
  printf("\n\x1b[1;32mExited Successfully from Ronto!\x1b[0m\n");
}

// Done

int enable_raw_mode(void) {
  struct termios raw;
  if (tcgetattr(term.fd, &raw) != 0) {
    perror("Failed to get terminal attributes");
    exit(1);
  }

  // Local Flag

  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

  // Input Flags

  raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON);

  // Output Flags

  raw.c_oflag &= ~(OPOST);

  // Control Flags

  raw.c_cflag |= (CS8); // No & mask, no bit flipping

  term.is_raw = 1;
  if (tcsetattr(term.fd, TCSAFLUSH, &raw) == -1) {
    editor_log("Failed to put the terminal in raw mode");
    perror("Failed to put the terminal in raw mode");
    return -1;
  }
  return 0;
}

// STATUS: Incomplete
void get_window_size(int *row, int *col) {
  struct winsize w;
  if (ioctl(term.fd, TIOCGWINSZ, &w) != 0) {
    editor_log("[ERROR] Failed to get the terminal window size");
    exit(1);
  }

  /*
  FOR DEBUG
  TODO: Clean Up after testing
  */
  *row = w.ws_row;
  *col = w.ws_col;
  editor_log("row: %d, col: %d", *row, *col);
}

// Collapse all `E->r`s to a single string
char *rowstostr_bf(ssize_t *s) {
  ssize_t size = 0;
  for (int i = 0; i < E.numrow; i++)
    size += E.r[i].size;
  *s = size;
  int lf_size = size - (E.numrow - 1);
  char *strs;
  strs = malloc(lf_size);
  assert(strs != NULL);

  for (int i = 0; i < E.numrow - 1; i++) {
    assert(E.r[i].content != NULL);
    memcpy(strs, E.r[i].content, E.r[i].size - 2);
    // crlf to lf
    strs[E.r[i].size - 1] = '\n';
    strs += E.r[i].size;
  }
  assert(E.r[E.numrow - 1].content != NULL);
  memcpy(strs, E.r[E.numrow - 1].content, E.r[E.numrow - 1].size);
  strs += E.r[E.numrow - 1].size;
  *strs = '\0';
  strs -= size;
  return strs;
}

char *rowstostr(ssize_t *s) {
  ssize_t size = 0;
  for (int i = 0; i < E.numrow; i++)
    size += E.r[i].size;
  *s = size;
  size -= E.numrow - 1;
  char *strs;
  strs = malloc(size);
  assert(strs != NULL);

  // LOG:
  editor_log("[INFO] size == %d\n", size);

  for (int i = 0; i < E.numrow - 1; i++) {
    assert(E.r[i].content != NULL);

    // insane mental and memory gymnastics, ik
    E.r[i].content[E.r[i].size - 2] = '\n';

    memcpy(strs, E.r[i].content, E.r[i].size - 1);

    strs += E.r[i].size - 1;
    E.r[i].content[E.r[i].size - 2] = '\r';
  }

  assert(E.r[E.numrow - 1].content != NULL);
  memcpy(strs, E.r[E.numrow - 1].content, E.r[E.numrow - 1].size);

  editor_log("[INFO]: strs == %s\n", strs);

  editor_log("[INFO]: E.numrow == %d\n", E.numrow);

  editor_log("[INFO]: E.r[E.numrow].size == %d\n", E.r[E.numrow - 1].size);

  strs += E.r[E.numrow - 1].size;

  *strs = '\0';

  editor_log("[INFO]: size == %d\n", size);

  strs -= size;

  editor_log("[INFO]: strs == %s\n", strs);

  return strs;
}

// DONE
void xclp_cpy(void) {
  ssize_t size = 0;
  char *s = rowstostr(&size);
  FILE *clipboard;
  // Use xclip for ~x11 and wl-copy for ~wayland
  if (!system("which xclip > /dev/null 2>&1")) {
    editor_log("Using xclip");
    clipboard = popen("xclip -selection clipboard > /dev/null", "w");
  } else {
    editor_log("Using wl-copy");
    clipboard = popen("wl-copy > /dev/null", "w");
  }
  fprintf(clipboard, "%s", s);

  // ignore returned value
  pclose(clipboard);

  return;
}

void save_file_temp(ssize_t size, char *strings) {
  (void)size;
  if (!E.temp_file) {
    E.temp_file = tmpfile();
  }
  fputs(strings, E.temp_file);
}

// DONE
void save_file(void) {
  if (E.file)
    rewind(E.file);
  if (E.save)
    return;
  if (!E.file) {
    if (E.file_name) {
      E.file = fopen(E.file_name, "w");
      rewind(E.file);
    } else {
      int fd = mkstemp(FILE_TEMPLATE);
      E.file = fdopen(fd, "w");
    }
  }
  ssize_t size = 0;
  char *strings = rowstostr(&size);
  // TODO: Error handeling
  if (fputs(strings, E.file) != 0) {
    editor_log("[ERROR]: Failed to save file: `fputs(strings, E.file) == 0`\n");
    editor_log("[INFO] strlen(strings) == %d\n", strlen(strings));
    // editor_log((char *)E.file);
  }
  return;
  E.save = true;
  free(strings);
  strings = NULL;
  fflush(E.file);
  editor_log("File saved");
  return;
}

void bootstrap_file(char *file) {
  FILE *f = fopen(file, "rb");
  struct stat fstat;
  size_t file_size;
  fseek(f, 0, SEEK_END);
  file_size = ftell(f);

  if (file_size < 1)
    return;

  char *lines = malloc(file_size);
  fseek(f, 0, SEEK_SET);
  fread(lines, 1, file_size, f);
  editor_log("%d\n", file_size);
  stat(file, &fstat);
  char *token = strtok(lines, "\r\n");
  int i = 0;
  E.y = 0;
  while (token != NULL) {
    int size = strlen(token);
    for (int i = 0; i < size; i++) {
      if (token[i] == '\t') {
        token[i] = ' ';
        token = realloc(token, size - 1);
        memmove(token + i + 2, token + i + 1, size + 1);
        token[i + 1] = ' ';
        size++;
      }
    }
    add_row(i, token, size);
    token = strtok(NULL, "\r\n");
    if (token != NULL) {
      E.r[i].content = realloc(E.r[i].content, size + 3);
      memcpy(E.r[i].content + size, "\r\n", 2);
      E.r[i].content[size + 2] = '\0';
      E.r[i].size += 2;
    }
    i++;
  }
  fclose(f);
}

// like buf but prints to the stdout immediate instead of storing
void bf_once(char *buf, ...) {
  int buf_len = strlen(buf);

  if (buf_len < 1)
    return;
  // Grab the variadic arguments
  va_list v;
  va_start(v, buf);

  char *f_buf = malloc(buf_len);
  vsprintf(f_buf, buf, v);
  if (!f_buf)
    return;
  write(STDOUT_FILENO, f_buf, buf_len);
  va_end(v);
}

// A buffer formatter. It appends messages to be sent to the std output, and
// flushes them all at once.
void bf(char *buf, ...) {
  int buf_len = strlen(buf);

  if (buf_len < 1)
    return;
  // Grab the variadic arguments
  va_list v;
  va_list v_copy;

  va_copy(v_copy, v);
  va_start(v, buf);

  int needed = vsnprintf(NULL, 0, buf, v_copy);
  va_end(v_copy);

  char *f_buf = malloc(needed + 1);
  vsprintf(f_buf, buf, v);

  if (!f_buf)
    return;
  b.seq = realloc(b.seq, b.l + buf_len);
  memcpy(b.seq + b.l, f_buf, buf_len);
  b.l += buf_len;
  free(f_buf);
  va_end(v);
}

void bf_flush(void) {
  if (b.seq == NULL)
    return;
  write(STDOUT_FILENO, b.seq, b.l);
  // Reset the append buffer
  free(b.seq);
  // TEST:
  // free(b.seq);
  b.seq = NULL;
  b.l = 0;
}

// TODO: Implement syntax highlighting here
// Highlighted keywords: 'int', 'char', 'float', 'struct', functions, bracket
// pairs((), {}, []).
int syntax_highlight_check(char *buf) {
  if (strcmp(buf, "return") == 0)
    return 1;
  if (strcmp(buf, "int") == 0)
    return 1;
  if (strcmp(buf, "float") == 0)
    return 1;
  if (strcmp(buf, "char") == 0)
    return 1;
  if (strcmp(buf, "static") == 0)
    return 1;
  if (strcmp(buf, "void") == 0)
    return 1;
  return 0;
}

int add_row(int pos, char *buf, ssize_t len) {
  E.r = realloc(E.r, sizeof(row) * (E.numrow + 1));
  // Error "Handeling"
  assert(E.r != NULL);

  E.r[pos].idx = pos;

  if (pos < E.numrow) {
    // FIX:  BUG
    memmove(E.r + pos + 2, E.r + pos + 1,
            sizeof(E.r[0]) * (E.numrow - pos - 1));
    E.r[pos + 1].content = malloc(len + 1);
    memcpy(E.r[pos + 1].content, buf, len);
    E.r[pos + 1].size = len;
    E.r[pos + 1].content[len] = '\0';
    E.numrow++;
    return 0;
  }
  E.r[pos].size = len;
  E.r[pos].content = malloc(len + 1);
  E.r[pos].render = NULL;
  E.r[pos].render_size = 0;
  E.numrow++;

  if (!buf) {
    E.r[pos].content[0] = '\0';
    return 0;
  }

  memcpy(E.r[pos].content, buf, len);
  editor_log("[INFO]: len == %d\n", len);
  E.r[pos].content[len] = '\0';

  editor_log("[INFO]: len == %d\n", len);
  editor_log("[INFO]: strlen(E.r[pos].content) == %d\n",
             strlen(E.r[pos].content));

  return 0;
}

void remove_row(int row) {
  // TODO: Make the code dry
  // if (!E.r[row].content) return; // Haha silly segfault
  if (E.r[row].size < 1) {
    E.x = E.r[row - 1].size - 2;
    E.y--;
    E.numrow--;
    row--;
    E.r[E.y].size -= 2;
    E.r[E.y].content = realloc(E.r[E.y].content, E.r[E.y].size);
    E.r = realloc(E.r, sizeof(E.r[0]) * E.numrow);
    return;
  }
  E.x = E.r[row - 1].size - 2; // WORKS!

  int res_size = E.r[row - 1].size + E.r[row].size - 2;
  E.r[row - 1].content = realloc(E.r[row - 1].content, res_size); // works
  memmove(E.r[row - 1].content + E.r[row - 1].size - 2, E.r[row].content,
          E.r[row].size);
  E.r[row - 1].size = res_size; // works

  // STUPID BUG: FIXED
  memmove(E.r + row, E.r + row + 1, sizeof(E.r[0]) * (E.numrow - row - 1));
  E.y--;
  E.numrow--;
  E.r = realloc(E.r, sizeof(E.r[0]) * E.numrow);
}

void add_char_at(char c, int at, int rowpos) {
  if (at < 0 || rowpos < 0) {
    editor_log("Error has occured. at=%d, pos=%d\n", at, rowpos);
    return;
  }
  if (!E.r[rowpos].content) {
    add_row(rowpos, "", 0);
    // E.r[rowpos].content = malloc(1);
    if (E.r[rowpos].content == NULL & E.y < E.numrow)
      E.r[rowpos].content = malloc(1);
    assert(E.r[rowpos].content != NULL);
  }
  int size = strlen(E.r[rowpos].content);
  // 1+ Hours of BUG.
  // E.r[rowpos].content = realloc(E.r[rowpos].content, E.r[rowpos].size + 1);
  E.r[rowpos].content = realloc(E.r[rowpos].content, size + 1);
  // move all the characters to the right when a character is added to the
  // middle of the content buffer.
  // E.r[rowpos].content[E.r[rowpos].size] = '\0';
  E.r[rowpos].content[size] = '\0';

  if (at < E.r[rowpos].size) {
    // memmove(E.r[rowpos].content + at + 1, E.r[rowpos].content + at,
    //         E.r[rowpos].size - at);
    memmove(E.r[rowpos].content + at + 1, E.r[rowpos].content + at, size - at);
  }
  E.r[rowpos].content[at] = c;
  // Add null char at the end
  E.r[rowpos].size++;
}

// TODO: move the `add_row` line to `add_char_at` function
// NOTE: Usage of E.x and E.y are perfectly valid. DONOT use E.rowoff and shit
void insert_key(char c) {
  if (iscntrl(c) && c != 9)
    return;
  if (E.x >= E.screencol) {
    E.coloff++;
    // E.x = 0;
  }
  if (E.y >= E.numrow) {
    add_row(E.y, "", 0);
    // E.y++;
  }
  add_char_at(c, E.x, E.y);
  E.x++;
}

// To insert tab, we just insert a user-defined
// tab-size amount of spaces in the buff.
// NOTE: use '\t' instead?
void insert_tab(void) {
  // for (int i = 0; i < E.tabsize; i++)
  //   insert_key(' ');
  insert_key(' ');
}

// Delete a character from the editor's text buff at position (rpos, at){(x,y)}
// TODO: Fix bugs
void delete_at(int rpos, int at) {
  if (rpos == 0 && at < 1) {
    return;
  };
  // TODO: Don't call realloc so often?
  if (at < E.r[rpos].size) {
    memmove(E.r[rpos].content + at, E.r[rpos].content + at + 1,
            E.r[rpos].size - at);
  }
  E.r[rpos].content = realloc(E.r[rpos].content, E.r[rpos].size + 1);
  E.r[rpos].content[E.r[rpos].size] = '\0';
}

void e_delete(void) {
  if (!E.r)
    return;
  // No Op if there is no character to remove
  if (E.y < 1 && E.x < 1) {
    return;
  }

  if (E.x < 1 && E.y > 0) {
    // if at the beginning of empty line and
    // delete if pressed, remove the whole empty line
    remove_row(E.y);
    return;
  }

  E.x--;
  E.r[E.y].size--;
  delete_at(E.y, E.x);
  if (E.coloff > 0) {
    E.coloff--;
  }
}

// Insert a new line between two lines
// if `cprogramminglanguage` and <enter> is pressed
// like `cproggraming<enter>language`, we get:
//      "cprogramming
//       |
//       language"
void enter_between(int row, int col) {
  int new_size = E.r[row].size - col;
  add_row(row, E.r[row].content + col, new_size);
  E.r[row].content = realloc(E.r[row].content, col + 3);
  memcpy(E.r[row].content + col, "\r\n", 2);
  E.r[row].size = col + 2;
  E.r[row].content[E.r[row].size] = '\0';
  E.y++;
  E.x = 0;
}

// TODO: Fix bugs
void enter_key(void) {
  // TODO: replace rp and cp
  int rp = E.y;
  // Handle in-between line enter key pressing
  int cp = E.x;
  // if (cp == 0) {

  if (!E.r) {
    add_row(rp, "\r\n", 2);
    // E.r[rp].content = NULL;
    // E.r[rp].content = realloc(E.r[rp].content, E.r[rp].size + 3);
    // memcpy(E.r[rp].content, "\r\n", 2);
    E.y++;
    E.x = 0;
    E.r[rp].size += 2;
    add_row(rp + 1, NULL, 0);
    // maybe remove this
    return;
  }
  // }
  if (cp < E.r[rp].size - 1) {
    editor_log("[INFO]: Enter Between Pressed\n");
    enter_between(rp, cp);
    return;
  } else {
    editor_log("[INFO]: New line created\n");
  }
  // TODO: remove this later
  // E.r[rp].content = NULL;

  E.r[rp].content = realloc(E.r[rp].content, E.r[rp].size + 3);
  assert(E.r[rp].content != NULL);
  int size = E.r[rp].size - cp;

  char *buf = NULL;

  // if (cp < size) {
  // SUGGESTED:
  if (cp < E.r[rp].size) {
    editor_log("[INFO]: cp < size\n");
    buf = E.r[rp].content + cp + 1;
  }
  // Append carriage return & newline to every row when enter key is pressed
  memcpy(E.r[rp].content + E.r[rp].size, "\r\n", 2);
  *(E.r[rp].content + (E.r[rp].size + 2)) = '\0';

  add_row(rp + 1, buf, size);
  E.y++;
  E.x = 0;
  E.r[rp].size += 2;
  return;
}

// move cursor to the current (x,y) coordinates
void shift_cursor(void) {
  if (!E.r)
    return;

  // Position the cursor at the current E.y row and E.x column
  if (E.y == E.numrow - 1 && E.x > E.r[E.y].size) {
    return;
  }
  bf("\x1b[%d;%dH", E.y + 1, E.x + 1);
  return;
}

// TODO: BUG
void arrow_key(int key) {

  if (!E.r)
    return;

  // TODO: Replace rp & cp with E.y and E.x
  int rp = E.y;
  int cp = E.x;

  // If at the end of everything, do nothing

  int row_factor = 0;
  int col_factor = 0;

  // Determine the type of key and execute
  // TODO: Make separate functions?

  if (key == ARROW_LEFT) {
    // If at the begginning of the editor, do nothing
    if (rp < 1 && cp < 1)
      return;
    if (cp < 1) {
      row_factor = -1;
      col_factor = -2;
      E.x = E.r[rp - 1].size;
    } else {
      // TODO: Move cursor by calculating E.r->size - E.x
      col_factor = -1;
    }

  }
  // TODO: Clear some rough edges
  else if (key == ARROW_RIGHT) {
    // If at the end of the editor lines, do nothing
    if (rp >= E.numrow - 1 && cp >= E.r[rp].size)
      return;

    if (rp < E.numrow - 1 && cp >= E.r[rp].size - 2) {
      row_factor = 1;
      col_factor = -E.x;

    } else {
      col_factor = 1;
      if (E.r[E.y].content[E.x] == '\t') {
        col_factor = 4;
      }
    }
  } else if (key == ARROW_UP) {
    // If trying to go up the first line, do nothing
    if (E.y < 1)
      return;

    row_factor = -1;
  } else if (key == ARROW_DOWN) {
    // If trying to go beyond the last line, do nothing
    if (E.y >= E.numrow - 1)
      return;
    row_factor = 1;
  }

  E.x += col_factor;
  E.y += row_factor;

  // The last line doesn't have a `\r\n` char.
  // So we don't subract 2 from the row size
  if (E.y >= E.numrow - 1) {
    if (E.x > E.r[E.y].size)
      E.x = E.r[E.y].size;
    return;
  }
  if (E.x >= E.r[E.y].size - 2)
    E.x = E.r[E.y].size - 2;
}

// Carriage Return + Newline
char *qmessage = "\r\nPressed Control + Q, so quitting\r\n";
char *smessage = "\r\nPressed Control + S, so saving\r\n";

// Maybe some editor config or other parameter
int key_up(void) {
  char seq[3];
  int num;

  /* Read three sequences from the terminal input. Useful to separate ESC
  key from arrow, home, end, et al. keys. It's like a timeout. Read 3 bytes,
  if only 1 byte is read, and the first byte is <ESC> key, switch to normal
  mode, else process the sequent bytes.
  */
  while ((num = read(term.fd, &seq, 3)) == 0)
    ;
  assert(num != -1);
  if (E.mode == INSERT)
    switch (seq[0]) {
    // DO Something
    case ESC:
      if (num == 1)
        return ESC;
      if (seq[1] != '[') {
        // DO SOMETHING
        // Page up/down, home, etc shise
      } else {
        switch (seq[2]) {
        case 'A':
          return ARROW_UP;

        case 'B':
          return ARROW_DOWN;
        case 'C':
          return ARROW_RIGHT;
        case 'D':
          return ARROW_LEFT;
        }
      }
      // Handle Escape Sequence
      return -1;
    case TAB:
      return TAB;
    default:
      return seq[0];
    }
  else
    switch (seq[0]) {
    case 'h':
      return ARROW_LEFT;
    case 'l':
      return ARROW_RIGHT;
    case 'j':
      return ARROW_DOWN;
    case 'k':
      return ARROW_UP;
    case 'i':
      E.mode = INSERT;
      break;
    case 'I':
      E.x = 0;
      E.mode = INSERT;
      break;
    case 'A':
      E.x = (E.y >= E.numrow - 1) ? E.r[E.y].size : E.r[E.y].size - 2;
      E.mode = INSERT;
      break;
    case 'd':
      // TODO: Implement remove line
    case 'x':
      return BACKSPACE;
      // TODO: remove this temporary "fix"
    default:
      return seq[0];
    }
  // no-op
  return -1;
}

// FEAT: rudimentary functionality complete
int handle_key_press(void) {
  int c = key_up();
  editor_log("[INFO] c == %c\n", c);
  switch (c) {
    // no-op
  case -1:
    return 0;
  case CTRL_Q:
    write(STDOUT_FILENO, qmessage, strlen(qmessage));
    exit(0);

  case CTRL_S:
    editor_log("%s", smessage);
    save_file();
    return 0;
  case CTRL_C:
    xclp_cpy();
    return 0;
  case ESC:
    E.mode = NORMAL;
    return 0;
  case ARROW_UP:
  case ARROW_DOWN:
  case ARROW_RIGHT:
  case ARROW_LEFT:
    arrow_key(c);
    return 0;

  case BACKSPACE:
    e_delete();
    E.save = false;
    return 1;
  case ENTER:
    enter_key();
    E.save = false;
    return 1;
  case TAB:
    insert_tab();
    E.save = false;
  default:
    if (E.mode == NORMAL)
      return 0;
    insert_key(c);
    E.save = false;
    // Handle default case i.e add it to the character buffer
    return 1;
  }
}

// TODO: Solve ugs
void expand_rows_temps(void) {
  int i = 0, y = 0, size, row_size = E.numrow;

  if (E.y >= E.screenrow) {
    i = E.y - E.screenrow;
  }
  if (E.numrow >= E.screenrow) {
    row_size = E.screenrow + i + 1;
  }
  if (E.x > E.screencol) {
    y = E.x - E.screencol + 1;
  }

  for (; i < row_size; i++) {
    if (E.r[i].content == NULL)
      return;
    // TODO: Fix bugs
    if ((E.x < E.screencol) && (E.r[i].size) > E.screencol) {
      size = E.screencol;
    } else if ((E.x >= E.screencol) && (E.r[i].size >= E.screencol)) {
      size = E.screencol;
    } else {
      size = E.r[i].size - y;
    }
    editor_log("[INFO] E.r[i].content + y == %s\n", E.r[i].content);
    write(STDOUT_FILENO, E.r[i].content + y, size);
    bf_flush();
  }
}

void expand_rows(void) {
  int i = 0, y = 0, size, row_size = E.numrow;

  // temp container to hold syntax-highlighted strings

  if (E.y >= E.screenrow) {
    i = E.y - E.screenrow;
  }
  if (E.numrow >= E.screenrow) {
    row_size = E.screenrow + i + 1;
  }
  if (E.x > E.screencol) {
    y = E.x - E.screencol + 1;
  }

  for (; i < row_size; i++) {

    if (E.r[i].content == NULL)
      return;
    // TODO: Fix bugs
    if ((E.x < E.screencol) && (E.r[i].size) > E.screencol) {
      size = E.screencol;
    } else if ((E.x >= E.screencol) && (E.r[i].size >= E.screencol)) {
      size = E.screencol;
    } else {
      size = E.r[i].size - y;
    }

    char *highlighted = NULL;
    // highlighted = malloc(size);

    // char *token;

    char *temp = malloc(strlen(E.r[i].content) + 1);
    strcpy(temp, E.r[i].content);
    temp[strlen(E.r[i].content)] = '\0';

    editor_log("[INFO]: E.r[i].content == %s\n", E.r[i].content);
    editor_log("strlen(E.r[i].content) == %d\n", strlen(E.r[i].content));

    // CHANGE:
    Token *t = token_tokenize(temp, ' ');

    char *token = NULL;

    while ((token = token_get_next(t)) != NULL) {
      editor_log("[INFO]: token == %s\n", token);
      if (syntax_highlight_check(token)) {
        highlighted = extend_string(highlighted, "\x1b[31m");
        highlighted = extend_string(highlighted, token);
        highlighted = extend_string(highlighted, "\x1b[0m");
      } else {
        highlighted = extend_string(highlighted, token);
      }
    }

    // highlighted = extend_string(highlighted, "\r\n");

    if (highlighted != NULL)
      write(STDOUT_FILENO, highlighted, strlen(highlighted));
    // temp[0] = '\0';

    // INFO: NEW LOGIC. May or May not work
    free(t);
    free(highlighted);
    free(temp);

    // t = NULL;
    highlighted = NULL;
    // temp = NULL;
    // free(temp);
  }
}

void refresh_screen(void) {
  /*
  As we move cursor to home each time during refresh, we just erase from
  current line i.e. the first, to the last line
  TODO: Modify for view buffers
  We can't directly flush everything after the expand_rows call. If we're to
  do so, we need to copy the string from content to the flush buffer.
  */

  char *write_buf = "\x1b[H\x1b[J";
  write(STDOUT_FILENO, write_buf, strlen(write_buf));
  // TODO: Expand tabs to spaces
  bf_once("\x1b[?25l"); // hide cursor
  expand_rows();

  // Shift cursor to the current postion
  shift_cursor();
  bf_once("\x1b[?25h"); // show cursor
  bf_flush();
  return;
}

// Program Driver

int main(int argc, char *argv[]) {
  // TODO: Accept only 1 argument(i.e. just the program name & nothing else)
  if (argc < 2) {
    printf("Error: \x1b[1;31mNo Arguments Supplied\x1b[0m\n");
    printf("Usage: ronto -[option] <file_name>\n");
    return -1;
  };
  char c;
  // bool file = 0;
  char *file_name = NULL;
  extern char *optarg;
  extern int optind;
  while ((c = getopt(argc, argv, "nlf:")) != -1) {
    switch (c) {
    case 'l':
      LOGGING = 1;

    case 'n':
      break;

    case 'f':
      // file = 1;
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
  // Goto alternate screen buffer & clear put the cursor to home position */
  bf("\x1b[?1049h\x1b[H");
  // bf("\x1b[7l");
  bf_flush();

  atexit(disable_raw_mode);

  init_editor(file_name);
  editor_log("%d [INFO]: Initiated Editor\n", time(NULL));
  // int x, y;
  enable_raw_mode();
  editor_log("%d [INFO]: Initiated raw mode\n", time(NULL));
  // int row, col;
  get_window_size(&E.screenrow, &E.screencol);
  // if (file) {
  //   bootstrap_file(file_name);
  // }
  // get_cursor_position(&y, &x);
  editor_log("[INFO] len == %d\n", token_len("hello"));

  while (1) {
    refresh_screen();
    handle_key_press();
    bf_flush();
  }
  return 0;
}
