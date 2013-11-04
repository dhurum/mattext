##Simple matrix-style pager

Shows text with matrix-style rain-of-symbols animation during page changes.  
Usage: `mattext [options] filename` or `cat filename | mattext [options]`.

###Options:
* `d`, `delay` - Delay between screen redraws, in ms, default 80;
* `l`, `rand_len` - Maximum length of random symbols columns;
* `o`, `onepage` - Show only one page, then exit;
* `n`, `non-interact` - Do not wait for user input before showing next page;
* `c`, `colorize` - Colorize output;
* `C`, `centrate-horiz` - Centrate text horizontally;
* `v`, `centrate-vert` - Centrate text vertically;

###Commands:
* `q` or `Q` - quit;
* any other keys - show next page;
