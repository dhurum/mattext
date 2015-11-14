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

#include "animation_fire.h"

void FireAnimation::init() {
  fire_heights.resize(terminal_width);
  fire_end_heights.resize(terminal_width);
  max_fire_height = terminal_height / 4;
  size_t prev = 1 + rand() % max_fire_height;

  for (auto &height : fire_heights) {
    height = prev;
    if (height == max_fire_height) {
      --height;
    } else {
      int seed = rand() % 100;
      if (seed <= 45) {
        ++height;
      } else if ((seed >= 55) && (height > 1)) {
        --height;
      }
    }
    prev = height;
  }

  max_fire_end_height = terminal_height / 8;
  prev = rand() % max_fire_end_height;

  for (auto &height : fire_end_heights) {
    height = prev;
    if (height == max_fire_end_height) {
      --height;
    } else {
      int seed = rand() % 100;
      if (seed <= 45) {
        ++height;
      } else if ((seed >= 55) && (height > 1)) {
        --height;
      }
    }
    prev = height;
  }

  tick_id = 0;
}

void FireAnimation::tick(ev::timer &w, int revents) {
  const int _terminal_height = static_cast<int>(terminal_height);
  bool stopped = true;
  size_t prev = fire_heights[0];
  size_t prev_end = fire_end_heights[0];

  for (size_t col = 0; col < terminal_width; ++col) {
    auto &height = fire_heights[col];
    int fire_start = terminal_height - 1 - tick_id - height + max_fire_height;
    auto &end_height = fire_end_heights[col];
    int fire_end = fire_start + height + end_height;

    if ((fire_end + 1) < 0) {
      continue;
    }
    stopped = false;

    if (fire_start >= _terminal_height) {
      continue;
    }

    if ((fire_start >= 0) && (fire_start < _terminal_height)) {
      terminal.set(col, fire_start, '|', false, ColorYellow, ColorRed);
    }

    if (((height  + end_height) >= 2) && ((fire_start + 2) >= 0)
        && ((fire_start + 2) < _terminal_height)) {
      terminal.set(col, fire_start + 2, '#', false, ColorRed, ColorYellow);
    }

    auto increased = increased_cols.find(col);
    if (increased != increased_cols.end()) {
      increased_cols.erase(increased);

      if (((fire_start + 1) >= 0) && ((fire_start + 1) < _terminal_height)) {
        terminal.set(col, fire_start + 1, '|', false, ColorYellow, ColorRed);
      }
      if (((height + end_height) >= 3) && ((fire_start + 3) >= 0)
          && ((fire_start + 3) < _terminal_height)) {
        terminal.set(col, fire_start + 3, '#', false, ColorRed, ColorYellow);
      }
    }

    if ((fire_end > 0) && (fire_end < (_terminal_height + 1))) {
      terminal.set(col, fire_end - 1, '~', false, ColorWhite, ColorBlack);
    }

    if ((fire_end >= 0) && (fire_end < _terminal_height)) {
      terminal.set(col, fire_end, text->get(col, fire_end), false, ColorDefault,
                   ColorDefault);
    }

    auto decreased = decreased_ends.find(col);
    if (decreased != decreased_ends.end()) {
      decreased_ends.erase(decreased);
      if (((fire_end + 1) >= 0) && ((fire_end + 1) < _terminal_height)) {
        terminal.set(col, fire_end + 1, text->get(col, fire_end + 1), false,
                     ColorDefault, ColorDefault);
      }
    }

    int seed = rand() % 100;
    if ((height <= prev) && (((prev - height) > 1)
                             || ((seed <= 55) && (height < max_fire_height)))) {
      ++height;
      increased_cols.insert(col);
    } else if ((height >= prev)
               && (((height - prev) > 1) || ((seed >= 45) && (height > 1)))) {
      --height;
    }
    prev = height;

    seed = rand() % 100;
    if ((end_height <= prev_end)
        && (((prev_end - end_height) > 1)
            || ((seed <= 15) && (end_height < max_fire_end_height)))) {
      ++end_height;
    } else if ((end_height >= prev_end)
               && (((end_height - prev_end) > 1)
                   || ((seed >= 85) && (end_height > 1)))) {
      --end_height;
      decreased_ends.insert(col);
    }
    prev_end = end_height;
  }

  ++tick_id;
  terminal.show();

  if (stopped) {
    stop();
  }
}
