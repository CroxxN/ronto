# A Work-in-Progress text editor

## TODO: 

- [ ] Fix text horizontal overflow bugs
- [ ] Fix general bugs and random segfaults
- [ ] Implement \t handeling *
- [ ] Add more vim keybindings
- [ ] UI*

* -> Important

## Features:

- Open, edit and save files
- Open a new file and start editing
- Line overflow handeling
- Row overflow handeling
- Minimal reliance on terminal escape sequnce (Deletion, movement, etc is implemented, uses terminal escape sequence to change position of cursor)

## How to use? 

- `git clone <this_repo>`
- `make` for build with no debug info
- or `make debug `for  build with debug info
- zsh```> ./ronto -n``` for scratch buffer
- zsh```> ./ronto -f <file_name>``` to open/create file
- `C-q` to quit, `C-s` to save

### Starts in vim mode. Hit `i` to start editing
