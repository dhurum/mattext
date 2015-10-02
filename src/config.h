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

#include <list>

#define DEFAULT_DELAY 80
#define DEFAULT_BLOCK_LINES 1

class Config {
 public:
  int delay = DEFAULT_DELAY;
  int rand_columns_len = 0;
  int block_lines = DEFAULT_BLOCK_LINES;
  bool noninteract = false;
  bool use_colors = false;
  bool center_horiz = false;
  bool center_horiz_longest = false;
  bool center_vert = false;
  bool without_japanese = false;
  bool infinite = false;
  std::list<const char *> files;

  Config(int argc, char *argv[]);
};
