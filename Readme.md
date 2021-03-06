## Simple matrix-style pager

Mattext shows text with different animations (including matrix-style rain-of-symbols) during page changes ([video](https://youtu.be/EuGp7daniX0)).  
Usage: `mattext [options] file1 [file2 ...]` or `cat file | mattext [options]`.

### Options:
* `-d`, `--delay <delay>` - Delay between screen redraws, in milliseconds, default 60;
* `-l`, `--rand-len <length>` - Maximum length of random symbols columns;
* `-n`, `--non-interact` - Do not wait for user input before showing next page;
* `-i`, `--infinite` - At the end of file start reading it from the beginning;
* `-b`, `--block-lines <lines>` - Block until at least specified number of lines is read, default 1;
* `-B`, `--block-page` - Block until full page is read;
* `-N`, `--no-color` - Do not colorize output;
* `-C`, `--center-horiz` - Center text horizontally;
* `-L`, `--center-horiz-longest` - Center text horizontally by longest string;
* `-v`, `--center-vert` - Center text vertically;
* `-e`, `--without-japanese` - Do not use Japanese symbols;
* `-a`, `--animation <name>` - Animation for switching pages. Available animations are matrix, reverse_matrix, none, fire, beam;
* `--animation-next <name>` - Animation for showing next page;
* `--animation-prev <name>` - Animation for showing previous page;
* `-t`, `--tab-width <width>` - Tab width, minimum 1, default 4;

### Commands:
* <kbd>q</kbd>, <kbd>ctrl + D</kbd> - Exit program;
* <kbd>f</kbd>, <kbd>j</kbd>, <kbd>space</kbd>, <kbd>PgDown</kbd>, <kbd>Down</kbd> - Show next page;
* <kbd>b</kbd>, <kbd>k</kbd>, <kbd>PgUp</kbd>, <kbd>Up</kbd> - Show previous page;

### Building:
You will need a c++ compiler with c++14 (c++1y) support, ncurses built with widechar support and libev.

If you use Archlinux, you can take stable [package](https://aur.archlinux.org/packages/mattext/) from aur, or PKGBUILD from [archlinux](https://github.com/dhurum/mattext/tree/archlinux) branch.  
If you use OSX, you can take homebrew formula from [osx](https://github.com/dhurum/mattext/tree/osx) branch.

To build mattext from sources open terminal, go to mattext dir, and run `mkdir build; cd build; cmake ../ && make`  
This will build mattext binary. To test it, you can run `./mattext ../src/mattext.cpp`  
If you would like to install mattext, you can run `make install`

### Examples:
* `mattext file` - Show file one page at a time, and exit at the end;
* `mattext -ni file` - Show file until exit key is pressed. When end is reached mattext starts reading it from the beginning;
* `tail -f file | mattext -n` - Show file, waiting for at least one new line added to it before redrawing screen;
* `echo "" | mattext -ni -b 0` - Show animation until quit command key is pressed, similar to cmatrix;
* `mattext -niLv dir/*` - Show all files from directory dir, centrating text horizontally by longest line and vertically, until exit key is pressed. When mattext reaches the end of the last file it starts reading the first file. This mode can be useful for showing off your ascii art collection;

You can redirect program output, in this case it would print text line by line, applying following transformations:  
* Break long lines so they fit in the terminal;
* Center text horizontally;
* Center text horizontally by longest string;
* Replace tabs with spaces;

### Troubleshooting:
If you see white squares instead of some symbols, there can be 3 options:

1. Your terminal does not support unicode - you can use another terminal or `--without-japanese` option;
2. You do not have any Japanese font - run `fc-list :lang=ja` and if this command shows nothing, install some. For example, Droid;
3. Your terminal can't find this font - add it to the terminal's fonts list. For example, urxvt config should look like `URxvt.font: xft:Liberation Mono:size=11, xft:Droid Sans Fallback:size=11`;
