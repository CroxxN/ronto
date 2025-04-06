# A minimal flush-or-no-flush terminal text editor

> [!WARNING]
> Not tested on Windows.


## TODO: 

- [X] Implement \t handeling *
- [ ] Add more vim keybindings
- [ ] UI*

* -> Important

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

- `git clone <url>`
- `make release` for build with no debug info
- or `make` for  build with debug info
- ```> ./ronto -n``` for scratch buffer
- ```> ./ronto -f <file_name>``` to open/create file
- ```> ./ronto -l{*}``` to enable logging
- `Control-q` to quit
- `Control-s` to save
- `Control-c` to copy to clipboard

