##Simple matrix-style pager

Shows text with matrix-style rain-of-symbols animation during page changes.  
Usage: `mattext [options] file1 [file2 ...]` or `cat file | mattext [options]`.

###Options:
* `d`, `delay` - Delay between screen redraws, in ms, default 80;
* `l`, `rand_len` - Maximum length of random symbols columns;
* `o`, `onepage` - Show only one page, then exit;
* `n`, `non-interact` - Do not wait for user input before showing next page;
* `i`, `infinite` - At the end of file start reading it from the beginning;
* `c`, `colorize` - Colorize output;
* `C`, `centrate-horiz` - Centrate text horizontally;
* `v`, `centrate-vert` - Centrate text vertically;
* `e`, `without-japanese` - Do not use Japanese symbols;

###Commands:
* `q` or `Q` - quit;
* any other keys - show next page;

###Troubleshooting:
If you see white squares instead of some symbols, there can be 3 options:

1. Your terminal does not support unicode - you can use another terminal or `--without-japanese` option;
2. You do not have any Japanese font - run `fc-list :lang=ja` and if this command shows nothing, install some. For example, Droid;
3. Your terminal can't find this font - add it to the terminal's fonts list. For example, urxvt config should look like `URxvt.font: xft:Liberation Mono:size=11, xft:Droid Sans Fallback:size=11`;
