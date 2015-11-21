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

#include "animation_none.h"
#include "terminal.h"
#include "file_reader.h"

NoneAnimation::NoneAnimation(const Terminal &terminal) : terminal(terminal) {}

void NoneAnimation::play(const Text &text, std::function<void()> on_stop) {
  for (size_t row = 0; row < terminal.getHeight(); ++row) {
    for (size_t col = 0; col < terminal.getWidth(); ++col) {
      terminal.set(col, row, text.get(col, row));
    }
  }
  terminal.show();
}

void NoneAnimation::stop() {}

bool NoneAnimation::isPlaying() {
  return false;
}
