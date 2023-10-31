/*
To use any function, anywhere without the compiler shouting
*/

#ifndef RONTO_H_
#define RONTO_H_

// We use the `vdprintf` function, which is a GNU Extension, and
// were described in POSIX.1-2008, so we define the POSIX C source
// to avoid `gcc` yelling about implicit functions
#define _POSIX_C_SOURCE 200809L

// Includes
#include <unistd.h>
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

// struct defs

// Vim-like mode of the editor
typedef enum {
  NORMAL,
  INSERT
}Mode;

// Describes one row of the Editor

typedef struct row {
  int idx;         // Current index of the row
  int size;        // Number of chars, punctuations and tabs in this current row
  int render_size; // size after adding all the tabs
  char *content;   // content of the row
  char *render;    // content ready for render - with tabs expanded
} row;


// Editor config & state

struct Editor {
  int x;
  int y;
  row *r;
  int numrow;
  int coloff;
  int rowoff;
  int screenrow;
  int screencol;
  bool save;
  // File stream of the temporary file used for saving data
  FILE *temp_file;

  // File stream of the opened file 
  FILE *file;

  FILE *log;
  Mode mode;
};

// a append buffer to flush everything at once
struct buf {
  char *seq; int l;
};

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
}; 

// Function defs: 

void editor_log(char *s, ...);
int isalnum_str(char *string);
int get_cursor_position(int *row, int *col);
int init_editor(char *file);
void disable_raw_mode(void);
int enable_raw_mode(void);
void query_cursor_pos(int *x, int *y);
void get_window_size(int *row, int *col);
char *rowstostr(ssize_t *s);
void xclp_cpy(void);
void save_file();
void bf(char *buf, ...);
void bf_flush(void);
int add_row(int pos, char* buf, ssize_t len);
void remove_row(int row);
void add_char_at(char c, int at, int rowpos);
void insert_key(char c);
void delete_at(int rpos, int at);
void e_delete(void);
void enter_key(void);
void enter_between(int row, int col);
void shift_cursor(void);
void set_cursor(int row, int pos);
void arrow_key(int key);
int key_up(void);
int handle_key_press(void);
void expand_rows(void);
void refresh_screen(void);


#endif
