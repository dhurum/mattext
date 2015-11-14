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

#include "animation_generic.h"

GenericAnimation::GenericAnimation(const Config &config, const Terminal &terminal)
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
