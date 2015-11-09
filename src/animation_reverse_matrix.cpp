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

#include "animation_reverse_matrix.h"

void ReverseMatrixAnimation::tick(ev::timer &w, int revents) {
  int _terminal_height = static_cast<int>(terminal_height);
  bool stopped = true;

  for (size_t col = 0; col < terminal.getWidth(); ++col) {
    int col_start = _terminal_height - 1 - tick_id + col_offsets[col];
    int col_end = col_start + col_lengths[col];
    int text_start = col_end + tail_length + 1;

    if ((col_start >= _terminal_height) || (text_start < 0)) {
      continue;
    }

    stopped = false;

    // Make previous bottom symbol not bold
    if ((col_start >= 0) && (col_start <= _terminal_height - 2)) {
      bool bold = ((rand() % 100) > 90) ? true : false;
      terminal.set(col, col_start + 1, terminal.get(col, col_start + 1), bold,
                   ColorGreen, ColorBlack);
    }

    // Place new random symbol to bottom of column
    if (col_start < _terminal_height) {
      terminal.set(col, col_start, getRandSymbol(), true, ColorGreen,
                   ColorBlack);
    }

    // Change random symbol in column
    int row_id = rand() % _terminal_height;
    if ((row_id >= col_end) && (row_id < col_start)) {
      bool bold = ((rand() % 100) > 60) ? true : false;
      terminal.set(col, row_id, getRandSymbol(), bold, ColorGreen, ColorBlack);
    }

    // Start showing some of real symbols in column
    int fade_row_id = (col_end + 1) + rand() % tail_length;
    if ((fade_row_id >= 0) && (fade_row_id < _terminal_height)) {
      terminal.set(col, fade_row_id, text->get(col, fade_row_id), false,
                   ColorGreen, ColorBlack);
    }

    // Show real text symbol
    if ((text_start >= 0) && (text_start < _terminal_height)) {
      terminal.set(col, text_start, text->get(col, text_start), false,
                   ColorGreen, ColorBlack);
    }
  }
  ++tick_id;
  terminal.show();

  if (stopped) {
    stop();
  }
}
