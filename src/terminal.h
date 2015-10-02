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
