##Simple matrix-style pager

Mattext shows text with matrix-style rain-of-symbols animation during page changes ([screenshots](http://dhurum.github.io/mattext/)).  
Usage: `mattext [options] file1 [file2 ...]` or `cat file | mattext [options]`.

###Options:
* `-d`, `--delay <delay>` - Delay between screen redraws, in milliseconds, default 80;
* `-l`, `--rand-len <length>` - Maximum length of random symbols columns;
* `-n`, `--non-interact` - Do not wait for user input before showing next page;
* `-i`, `--infinite` - At the end of file start reading it from the beginning;
* `-b`, `--block-lines <lines>` - Block until at least specified number of lines is read, default 1;
* `-B`, `--block-page` - Block until full page is read;
* `-c`, `--colorize` - Colorize output (green text on black background);
* `-C`, `--center-horiz` - Center text horizontally;
* `-L`, `--center-horiz-longest` - Center text horizontally by longest string;
* `-v`, `--center-vert` - Center text vertically;
* `-e`, `--without-japanese` - Do not use Japanese symbols;

###Commands:
* <kbd>q</kbd>, <kbd>Q</kbd>, <kbd>ctrl + D</kbd> - Exit program;
* <kbd>f</kbd>, <kbd>F</kbd>, <kbd>space</kbd> - Show next page;
* <kbd>b</kbd>, <kbd>B</kbd> - Show previous page;

###Building:
If you use Archlinux, you can take [package](https://aur.archlinux.org/packages/mattext/) from aur, or PKGBUILD from [archlinux](https://github.com/dhurum/mattext/tree/archlinux) branch.  
If you use OSX, you can take homebrew formula from [osx](https://github.com/dhurum/mattext/tree/osx) branch.

To build mattext from sources open terminal, go to mattext dir, and run `mkdir build; cd build; cmake ../ && make`.  
This will build mattext binary. To test it, you can run `./mattext -c ../mattext.cpp`.
If you would like to install mattext, you can run `make install`.

###Examples:
* `mattext -c file` - Show file, colorizing console, one page at a time, and exit at the end;
* `mattext -cni file` - Show file, colorizing console, until exit key is pressed. When end is reached mattext starts reading it from the beginning;
* `tail -f file | mattext -n` - Show file, waiting for at least one new line added to it before redrawing screen;
* `echo "" | mattext -ni -b 0` - Show animation until quit command key is pressed, similar to cmatrix;
* `mattext -niLv dir/*` - Show all files from directory dir, centrating text horizontally by longest line and vertically, until exit key is pressed. When mattext reaches the end of the last file it starts reading the first file. This mode can be useful for showing off your ascii art collection;

###Troubleshooting:
If you see white squares instead of some symbols, there can be 3 options:

1. Your terminal does not support unicode - you can use another terminal or `--without-japanese` option;
2. You do not have any Japanese font - run `fc-list :lang=ja` and if this command shows nothing, install some. For example, Droid;
3. Your terminal can't find this font - add it to the terminal's fonts list. For example, urxvt config should look like `URxvt.font: xft:Liberation Mono:size=11, xft:Droid Sans Fallback:size=11`;
