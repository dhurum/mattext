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

#pragma once

#include <ev++.h>
#include <unistd.h>
#include <functional>
#include <map>
#include <vector>

class Config;

enum Colors {
  ColorDefault = -1,
  ColorBlack,
  ColorRed,
  ColorGreen,
  ColorYellow,
  ColorBlue,
  ColorMagenta,
  ColorCyan,
  ColorWhite
};

class Terminal {
 public:
  Terminal(const Config &config);
  ~Terminal();
  size_t getWidth() const;
  size_t getHeight() const;
  bool stdoutIsTty() const;
  bool stdinIsTty() const;
  void set(int column, int row, wchar_t symbol, bool bold = false,
           short fg = ColorDefault, short bg = ColorDefault) const;
  wchar_t get(int column, int row) const;
  void setColors(short fg, short bg) const;
  void show() const;
  void clear() const;
  int stdinFd() const;
  void onKeyPress(std::function<void(int)> on_key) const;
  void stop() const;

 private:
  int width;
  int height;
  bool stdout_is_tty = true;
  bool stdin_is_tty = true;
  bool use_colors = false;
  short default_fg = -1;
  short default_bg = -1;
  std::vector<short> colors;
  mutable std::map<std::pair<short, short>, short> color_pairs;
  int tty_fd;
  int stdin_fd = STDIN_FILENO;
  mutable ev::io io_watcher;
  mutable std::vector<std::function<void(int)>> on_key_press;

  short getColor(short color) const;
  short getColorPair(short fg, short bg) const;
  void inputCb(ev::io &w, int revents);
};
