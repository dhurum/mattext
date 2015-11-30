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
#include "animation_generic.h"
#include "terminal.h"
#include "config.h"

GenericAnimation::GenericAnimation(const Config &config,
                                   const Terminal &terminal)
    : config(config), terminal(terminal) {}

void GenericAnimation::play(const Text &_text, std::function<void()> _on_stop) {
  text = &_text;
  on_stop = _on_stop;
  terminal_width = terminal.getWidth();
  terminal_height = terminal.getHeight();

  init();
  is_playing = true;
  timer_watcher.set<GenericAnimation, &GenericAnimation::tick>(this);
  timer_watcher.start(0., static_cast<double>(config.delay) / 1000.0);
}

void GenericAnimation::stop() {
  is_playing = false;
  timer_watcher.stop();
  if (on_stop) {
    on_stop();
  }
}

bool GenericAnimation::isPlaying() {
  return is_playing;
}

void GenericAnimation::drawCircle(int radius, int center_x, int center_y,
                               bool bold, short color) {
  for (int quad = 0; quad < 4; ++quad) {
    int x_mul = -1;
    if (quad % 2) {
      x_mul = 1;
    }
    int y_mul = -1;
    if (quad >= 2) {
      y_mul = 1;
    }
    int prev_h = 0;
    for (int i = 0; i <= radius; ++i) {
      const int x = center_x + (i * 2 * x_mul);
      const int h = sqrt(radius * radius - i * i);
      const int y = center_y + (h * y_mul) + ((quad >= 2) ? 1 : 0);

      terminal.set(x - 1 + x_mul, y, ' ', false, color, color);
      terminal.set(x + x_mul, y, ' ', false, color, color);
      if (prev_h) {
        if ((prev_h - h) > 1) {
          for (int k = 1; k <= (prev_h - h); ++k) {
            terminal.set(x - 1 - x_mul, y + (k * y_mul), ' ', false, color,
                         color);
            terminal.set(x - x_mul, y + (k * y_mul), ' ', false, color, color);
          }
        }
        if (bold && ((prev_h - h) >= 1)) {
          terminal.set(x - 1 - x_mul, y, ' ', false, color, color);
          terminal.set(x - x_mul, y, ' ', false, color, color);
        }
      }
      prev_h = h;
    }
  }
}

// Algorithm taken from
// http://www.roguebasin.com/index.php?title=Bresenham%27s_Line_Algorithm
void GenericAnimation::drawLine(int x1, int y1, int x2, int y2, short color,
                             std::function<wchar_t(int, int)> callback) {
  int x_delta = x2 - x1;
  int y_delta = y2 - y1;
  int x_inc = (x_delta > 0) ? 1 : ((x_delta < 0) ? -1 : 0);
  int y_inc = (y_delta > 0) ? 1 : ((y_delta < 0) ? -1 : 0);
  x_delta = abs(x_delta * 2);
  y_delta = abs(y_delta * 2);
  if (!callback) {
    callback = [](int, int) { return L' '; };
  }

  terminal.set(x1, y1, callback(x1, y1), false, color, color);

  if (x_delta >= y_delta) {
    int error = y_delta - (x_delta / 2);

    while (x1 != x2) {
      if ((error >= 0) && (error || (x_inc > 0))) {
        error -= x_delta;
        y1 += y_inc;
      }
      error += y_delta;
      x1 += x_inc;
      terminal.set(x1, y1, callback(x1, y1), false, color, color);
    }
  } else {
    int error = x_delta - (y_delta / 2);

    while (y1 != y2) {
      if ((error >= 0) && (error || (y_inc > 0))) {
        error -= y_delta;
        x1 += x_inc;
      }
      error += x_delta;
      y1 += y_inc;
      terminal.set(x1, y1, callback(x1, y1), false, color, color);
    }
  }
}
