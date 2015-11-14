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

#include "animation.h"

class GenericAnimation : public Animation {
 public:
  GenericAnimation(const Config &config, const Terminal &terminal);
  void play(const Text &text, std::function<void()> on_stop) override;
  void stop() override;
  bool isPlaying() override;

 protected:
  const Config &config;
  const Terminal &terminal;
  const Text *text;
  ev::timer timer_watcher;
  bool is_playing = false;
  size_t terminal_width;
  size_t terminal_height;
  std::function<void()> on_stop;

  virtual void init() = 0;
  virtual void tick(ev::timer &w, int revents) = 0;
};
