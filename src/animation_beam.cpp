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

#include <math.h>
#include "animation_beam.h"
#include "terminal.h"
#include "config.h"
#include "file_reader.h"

void BeamAnimation::init() {
  tick_id = 0;
  showing_flash = true;
  showing_beam = false;
  showing_text = false;
  flash_max_radius = terminal_height / 8;
  flash_radius = 1;
  flash_color = ColorWhite;
  flash_symbol = L' ';
  beam_height = 0;
  beam_step = 0;
  text_edges.resize(terminal_height);

  for (auto &edge : text_edges) {
    edge = std::make_pair<int, int>(-1, -1);
  }

  text_show_cb = [this](int x, int y) {
    if (y == terminal_height - 1) {
      return L' ';
    }
    if (x >= (terminal_width / 2)) {
      int edge = text_edges[y].second;
      if (edge == -1) {
        edge = terminal_width / 2;
      }
      for (int i = edge; i < x; ++i) {
        terminal.set(i, y, text->get(i, y), false, ColorBlack, ColorWhite);
      }
      text_edges[y].second = x;
    } else {
      int edge = text_edges[y].first;
      if (edge == -1) {
        edge = terminal_width / 2 - 1;
      }
      for (int i = x + 1; i <= edge; ++i) {
        terminal.set(i, y, text->get(i, y), false, ColorBlack, ColorWhite);
      }
      text_edges[y].first = x;
    }

    return L'|';
  };

  terminal.setColors(ColorBlack, ColorBlack);
  for (int x = 0; x < terminal_width; ++x) {
    for (int y = 0; y < terminal_height; ++y) {
      terminal.set(x, y, '#', false, ColorBlack, ColorBlack);
    }
  }
}

void BeamAnimation::tick(ev::timer &w, int revents) {
  const int center_x = terminal_width / 2;
  const int center_y = terminal_height - 1;

  if (showing_text) {
    int beam_1_x = 0;
    int beam_1_y = 0;
    int beam_2_x = 0;
    int beam_2_y = 0;

    switch (beam_step) {
      case 0:
        beam_1_x = terminal_width * 3 / 4;
        beam_2_x = terminal_width / 4;
        break;
      case 1:
        beam_1_x = terminal_width - 1;
        break;
      case 2:
        beam_1_x = terminal_width - 1;
        beam_1_y = terminal_height / 2;
        beam_2_y = terminal_height / 2;
        for (int y = 0; y < terminal_height / 2; ++y) {
          for (int i = 0; i <= text_edges[y].first; ++i) {
            terminal.set(i, y, text->get(i, y), false, ColorBlack, ColorWhite);
          }
          for (int i = text_edges[y].second; i <= terminal_width - 1; ++i) {
            terminal.set(i, y, text->get(i, y), false, ColorBlack, ColorWhite);
          }
        }
        break;
      case 3:
        beam_1_x = terminal_width - 1;
        beam_1_y = center_y;
        beam_2_y = center_y;
        for (int y = terminal_height / 2; y < center_y; ++y) {
          for (int i = 0; i <= text_edges[y].first; ++i) {
            terminal.set(i, y, text->get(i, y), false, ColorBlack, ColorWhite);
          }
          for (int i = text_edges[y].second; i < terminal_width; ++i) {
            terminal.set(i, y, text->get(i, y), false, ColorBlack, ColorWhite);
          }
        }
        break;
      default:
        for (int i = 0; i < terminal_width; ++i) {
          terminal.set(i, center_y, text->get(i, center_y), false, ColorBlack,
                       ColorWhite);
        }
        showing_text = false;
        break;
    }
    if (beam_step < 4) {
      drawLine(center_x, center_y, beam_1_x, beam_1_y, ColorCyan, text_show_cb);
      drawLine(center_x - 1, center_y, beam_2_x, beam_2_y, ColorCyan,
               text_show_cb);
      ++beam_step;
    }
  }
  if (showing_flash) {
    drawCircle(flash_radius, center_x, center_y, true, flash_color,
               flash_symbol);
    drawCircle(flash_radius + 1, center_x, center_y, true, flash_color,
               flash_symbol);

    if (flash_color != ColorBlack) {
      drawCircle(flash_radius + 1, center_x, center_y, false, ColorCyan, L'o');
      drawCircle(flash_radius - 1, center_x, center_y, false, flash_color,
                 flash_symbol);
    } else if (flash_radius > 0) {
      drawCircle(flash_radius - 1, center_x, center_y, false, ColorCyan);
    }

    if ((tick_id <= (flash_max_radius / 2))) {
      flash_radius += 2;
    } else if (tick_id == ((flash_max_radius / 2) + 1)) {
      flash_color = ColorBlack;
      flash_symbol = L'#';
      showing_beam = true;
    } else {
      flash_radius -= 2;
    }
    if (flash_radius <= 0) {
      showing_flash = false;
    }
  }
  if (showing_beam && (tick_id > ((flash_max_radius / 2) + 1))) {
    beam_height += (terminal_height / flash_max_radius) + 1;

    drawLine(center_x, center_y - flash_radius - 1, center_x,
             center_y - beam_height, ColorCyan, [](int, int) { return L'|'; });
    drawLine(center_x - 1, center_y - flash_radius - 1, center_x - 1,
             center_y - beam_height, ColorCyan, [](int, int) { return L'|'; });

    if (beam_height >= terminal_height) {
      showing_beam = false;
      showing_text = true;
    }
  }

  ++tick_id;
  terminal.show();

  if (!showing_flash && !showing_beam && !showing_text) {
    stop();
  }
}
