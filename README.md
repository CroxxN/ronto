# A minimal flush-or-no-flush syntax-highlighting terminal text editor

> [!WARNING]
> Not tested on Windows.<br>
> Starts in Vim mode. Hit `i` to start editing

​Ronto is a minimalistic terminal-based text editor with rudimentary syntax-highlighting designed to operate with minimal reliance on terminal escape sequences. It offers a subset of Vim keybindings, including hjkl for navigation, i for entering insert mode, and Escape to return to normal mode. Users can open, edit, and save files, as well as create new files from scratch. Ronto handles line and row overflows and implements custom deletion and movement functionalities.

<br/>

![image](https://github.com/user-attachments/assets/0e5dec9e-2106-4447-823a-9417b762984a)


## Features:

- Open, edit and save files
- Syntax highlighting for a select C keywords and types
- A subset of Vim keybindings (`hjkl` sequence, `escape`, and `i` implemented)
- Line overflow handling
- Row overflow handling
- Minimal reliance on terminal escape sequence (Deletion, movement, etc is custom implemented, only uses terminal escape sequence to change position of cursor)

## How to use? 


#### Requirements: A C compiler (gcc or clang) and `make`

- Clone this repository
```bash
> git clone <url>
```

- To build with no debug info
```bash
> make release
```
- OR to build with debug info
```bash
> make
```
- To start with a new scratch buffer
```bash
  > ./ronto -n
```
-  To open/create file:
```bash
> ./ronto -f <file_name>
```
- To enable logging
```bash
> ./ronto -l {*}
```
- To clean the log file and the binary
```bash
> make clean
```

- `Control-q` to quit
- `Control-s` to save
- `Control-c` to copy to clipboard
  
## TODO: 

- [X] Implement \t handeling
- [ ] Add more vim keybindings
- [ ] UI
