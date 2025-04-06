# A minimal flush-or-no-flush terminal text editor
> [!WARNING]
> Not tested on Windows.

​Ronto is a minimalistic terminal-based text editor designed to operate with minimal reliance on terminal escape sequences. It offers a subset of Vim keybindings, including hjkl for navigation, i for entering insert mode, and Escape to return to normal mode. Users can open, edit, and save files, as well as create new files from scratch. Ronto handles line and row overflows and implements custom deletion and movement functionalities.

## Features:

- Open, edit and save files
- Open a new file and start editing
- A subset of Vim keybindings (`hjkl` sequence, `escape`, and `i` implemented)
- Line overflow handeling
- Row overflow handeling
- Minimal reliance on terminal escape sequnce (Deletion, movement, etc is custom implemented, only uses terminal escape sequence to change position of cursor)

## How to use? 

> [!Warning]
> Starts in vim mode. Hit `i` to start editing

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
> ./ronto -l{*}
``` 
- `Control-q` to quit
- `Control-s` to save
- `Control-c` to copy to clipboard

## TODO: 

- [X] Implement \t handeling
- [ ] Add more vim keybindings
- [ ] UI
