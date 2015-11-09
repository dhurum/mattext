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

#include "animation_matrix.h"

MatrixAnimation::MatrixAnimation(const Config &config, const Terminal &terminal)
    : config(config), terminal(terminal) {
  if (config.rand_columns_len <= 0) {
    max_col_length = terminal.getHeight();
  } else {
    max_col_length = config.rand_columns_len;
  }
}

void MatrixAnimation::play(const Text &_text, std::function<void()> _on_stop) {
  text = &_text;
  on_stop = _on_stop;
  init();
  is_playing = true;
  timer_watcher.set<MatrixAnimation, &MatrixAnimation::tick>(this);
  timer_watcher.start(0., static_cast<double>(config.delay) / 1000.0);
}

void MatrixAnimation::stop() {
  is_playing = false;
  timer_watcher.stop();
  if (on_stop) {
    on_stop();
  }
}

bool MatrixAnimation::isPlaying() {
  return is_playing;
}

void MatrixAnimation::init() {
  col_lengths.resize(terminal.getWidth());
  col_offsets.resize(terminal.getWidth());
  tick_id = 0;

  // Random symbols are organized into columns
  for (size_t i = 0; i < terminal.getWidth(); ++i) {
    col_lengths[i] = 1 + rand() % max_col_length;
    col_offsets[i] = rand() % max_col_length;
  }
  col_offsets[rand() % terminal.getWidth()] = 0;
  terminal.setColors(ColorGreen, ColorBlack);
}

void MatrixAnimation::tick(ev::timer &w, int revents) {
  int terminal_height = static_cast<int>(terminal.getHeight());
  bool stopped = true;

  for (size_t col = 0; col < terminal.getWidth(); ++col) {
    int col_start = tick_id - col_offsets[col];
    int col_end = col_start - col_lengths[col];
    int text_start = col_end - tail_length - 1;

    if ((col_start < 0) || (text_start >= terminal_height)) {
      continue;
    }

    stopped = false;

    // Make previous bottom symbol not bold
    if ((col_start >= 1) && (col_start <= terminal_height)) {
      bool bold = ((rand() % 100) > 90) ? true : false;
      terminal.set(col, col_start - 1, terminal.get(col, col_start - 1), bold,
                   ColorGreen, ColorBlack);
    }

    // Place new random symbol to bottom of column
    if (col_start < terminal_height) {
      terminal.set(col, col_start, getRandSymbol(), true, ColorGreen,
                   ColorBlack);
    }

    // Change random symbol in column
    int row_id = rand() % terminal_height;
    if ((row_id >= col_end) && (row_id < col_start)) {
      bool bold = ((rand() % 100) > 60) ? true : false;
      terminal.set(col, row_id, getRandSymbol(), bold, ColorGreen, ColorBlack);
    }

    // Start showing some of real symbols in column
    int fade_row_id = (col_end - 1) - rand() % tail_length;
    if ((fade_row_id >= 0) && (fade_row_id < terminal_height)) {
      terminal.set(col, fade_row_id, text->get(col, fade_row_id), false,
                   ColorGreen, ColorBlack);
    }

    // Show real text symbol
    if ((text_start >= 0) && (text_start < terminal_height)) {
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

wchar_t MatrixAnimation::getRandSymbol() {
  int type = rand() % (config.without_japanese ? 600 : 1000);

  if (type < 100) {
    return 0x30 + rand() % 9;
  } else if (type < 300) {
    return 0x41 + rand() % 26;
  } else if (type < 600) {
    return 0x61 + rand() % 26;
  }
  return 0xff66 + rand() % 58;
}