/*******************************************************************************

Copyright 2015 Denis Tikhomirov

This file is part of Mattext

Mattext is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Mattext is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with Mattext.  If not, see http://www.gnu.org/licenses/.

*******************************************************************************/

#define _XOPEN_SOURCE_EXTENDED
#include <curses.h>
#include <vector>
#include <unistd.h>
#include <sstream>
#include <stdexcept>
#include <assert.h>
#include <sys/ioctl.h>
#include <string.h>
#include "terminal.h"

Terminal::Terminal(const Config &config)
    : colors{COLOR_BLACK, COLOR_RED,     COLOR_GREEN, COLOR_YELLOW,
             COLOR_BLUE,  COLOR_MAGENTA, COLOR_CYAN,  COLOR_WHITE} {
  if (!isatty(fileno(stdout))) {
    struct winsize w_size;
    if (ioctl(0, TIOCGWINSZ, &w_size) == -1) {
      std::ostringstream err;
      err << "Can't get terminal size via ioctl: " << strerror(errno);
      throw std::runtime_error(err.str());
    }
    is_tty = false;
    width = w_size.ws_col;
    height = w_size.ws_row;

    return;
  }

  initscr();
  getmaxyx(stdscr, height, width);
  noecho();
  cbreak();
  curs_set(0);

  if (has_colors() && config.use_colors) {
    use_colors = true;
    start_color();
    use_default_colors();

    // Workaround for ncruses bug, bkgd does not work if called right after
    // use_default_colors.
    refresh();

    color_pairs[std::make_pair(default_fg, default_bg)] = 0;
  }
}

Terminal::~Terminal() {
  if (is_tty) {
    endwin();
  }
}

size_t Terminal::getWidth() const {
  return width;
}

size_t Terminal::getHeight() const {
  return height;
}

bool Terminal::isTty() {
  return is_tty;
}

short Terminal::getColor(short color) const {
  if ((color < 0) || (static_cast<size_t>(color) >= colors.size())) {
    throw std::runtime_error(
        "Can't add another color, maximum number was reached");
  }
  return colors[color];
}

short Terminal::getColorPair(short fg, short bg) const {
  short color_pair;

  if (fg == ColorDefault) {
    fg = default_fg;
  } else {
    fg = getColor(fg);
  }
  if (bg == ColorDefault) {
    bg = default_bg;
  } else {
    bg = getColor(bg);
  }
  try {
    color_pair = color_pairs.at(std::make_pair(fg, bg));
  } catch (std::out_of_range) {
    color_pair = color_pairs.size();
    if (color_pair >= COLOR_PAIRS) {
      throw std::runtime_error(
          "Can't add another color pair, maximum number was reached");
    }
    init_pair(color_pair, fg, bg);
    color_pairs[std::make_pair(fg, bg)] = color_pair;
  }

  return color_pair;
}

void Terminal::set(size_t column, size_t row, wchar_t symbol, bool bold,
                   short fg, short bg) const {
  assert(is_tty);
  wchar_t str[] = {symbol, L'\0'};
  cchar_t cchar;
  attr_t attr = bold ? A_BOLD : A_NORMAL;
  short color_pair = 0;

  if (use_colors) {
    color_pair = getColorPair(fg, bg);
  }

  setcchar(&cchar, str, attr, color_pair, nullptr);
  mvadd_wch(row, column, &cchar);
}

wchar_t Terminal::get(size_t column, size_t row) const {
  assert(is_tty);
  cchar_t cchar;

  mvin_wch(row, column, &cchar);

  std::vector<wchar_t> str(
      getcchar(&cchar, nullptr, nullptr, nullptr, nullptr));
  attr_t attr;
  short color_pair;

  getcchar(&cchar, str.data(), &attr, &color_pair, nullptr);

  return str[0];
}

void Terminal::setColors(short fg, short bg) const {
  assert(is_tty);
  if (!use_colors) {
    return;
  }

  bkgd(COLOR_PAIR(getColorPair(fg, bg)));
  show();
}

void Terminal::show() const {
  assert(is_tty);
  refresh();
}

void Terminal::clear() const {
  assert(is_tty);
  erase();
}
