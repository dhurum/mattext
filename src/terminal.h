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

#include <vector>
#include <map>
#include "config.h"

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
  bool isTty();
  void set(size_t column, size_t row, wchar_t symbol, bool bold = false,
           short fg = ColorDefault, short bg = ColorDefault) const;
  wchar_t get(size_t column, size_t row) const;
  void setColors(short fg, short bg) const;
  void show() const;
  void clear() const;

 private:
  size_t width;
  size_t height;
  bool is_tty = true;
  bool use_colors = false;
  short default_fg = -1;
  short default_bg = -1;
  std::vector<short> colors;
  mutable std::map<std::pair<short, short>, short> color_pairs;

  short getColor(short color) const;
  short getColorPair(short fg, short bg) const;
};
