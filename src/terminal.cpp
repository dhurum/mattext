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
#include <fcntl.h>
#include "terminal.h"
#include "config.h"

Terminal::Terminal(const Config &config)
    : colors{COLOR_BLACK, COLOR_RED,     COLOR_GREEN, COLOR_YELLOW,
             COLOR_BLUE,  COLOR_MAGENTA, COLOR_CYAN,  COLOR_WHITE} {
  if (!isatty(STDOUT_FILENO)) {
    struct winsize w_size;
    if (ioctl(0, TIOCGWINSZ, &w_size) == -1) {
      std::ostringstream err;
      err << "Can't get terminal size via ioctl: " << strerror(errno);
      throw std::runtime_error(err.str());
    }
    stdout_is_tty = false;
    width = w_size.ws_col;
    height = w_size.ws_row;

    return;
  }

  tty_fd = open("/dev/tty", O_RDONLY | O_NONBLOCK);
  if (tty_fd == -1) {
    std::ostringstream err;
    err << "Can't open file '/dev/tty': " << strerror(errno);
    throw std::runtime_error(err.str());
  }

  if (!isatty(STDIN_FILENO)) {
    stdin_fd = dup(STDIN_FILENO);
    if ((stdin_fd == -1) || (dup2(tty_fd, STDIN_FILENO) == -1)) {
      std::ostringstream err;
      err << "Can't redirect stdin to '/dev/tty': " << strerror(errno);
      throw std::runtime_error(err.str());
    }
    stdin_is_tty = false;
  }

  initscr();
  getmaxyx(stdscr, height, width);
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, true);

  if (has_colors() && config.use_colors) {
    use_colors = true;
    start_color();
    use_default_colors();

    // Workaround for a bug in old ncurses versions, bkgd does not work if
    // called right after use_default_colors.
    refresh();

    color_pairs[std::make_pair(default_fg, default_bg)] = 0;
  }
  onKeyPress([this](int cmd) {
    if (cmd == KEY_RESIZE) {
      getmaxyx(stdscr, height, width);
    }
  });
}

Terminal::~Terminal() {
  if (stdout_is_tty) {
    endwin();
  }
  if (!stdin_is_tty) {
    close(stdin_fd);
  } else {
    close(tty_fd);
  }
  stop();
}

size_t Terminal::getWidth() const {
  return width;
}

size_t Terminal::getHeight() const {
  return height;
}

bool Terminal::stdoutIsTty() const {
  return stdout_is_tty;
}

bool Terminal::stdinIsTty() const {
  return stdin_is_tty;
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

void Terminal::set(int column, int row, wchar_t symbol, bool bold, short fg,
                   short bg) const {
  assert(stdout_is_tty);
  if ((column < 0) || (column >= width) || (row < 0) || (row >= height)) {
    return;
  }
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

wchar_t Terminal::get(int column, int row) const {
  assert(stdout_is_tty);
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
  assert(stdout_is_tty);
  if (!use_colors) {
    return;
  }

  bkgd(COLOR_PAIR(getColorPair(fg, bg)));
  show();
}

void Terminal::show() const {
  assert(stdout_is_tty);
  refresh();
}

void Terminal::clear() const {
  assert(stdout_is_tty);
  erase();
}

int Terminal::stdinFd() const {
  return stdin_fd;
}

void Terminal::stop() const {
  io_watcher.stop();
}

void Terminal::inputCb(ev::io &w, int revents) {
  int key = getch();
  for (auto cb : on_key_press) {
    cb(key);
  }
}

void Terminal::onKeyPress(std::function<void(int)> _on_key_press) const {
  if (!_on_key_press) {
    return;
  }
  if (!on_key_press.size()) {
    io_watcher.set<Terminal, &Terminal::inputCb>(const_cast<Terminal *>(this));
    io_watcher.start(tty_fd, ev::READ);
  }
  on_key_press.push_back(_on_key_press);
}
