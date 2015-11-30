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

#include <vector>
#include "animation_generic.h"

class BeamAnimation : public GenericAnimation {
 public:
  using GenericAnimation::GenericAnimation;

 private:
  int tick_id;
  bool showing_flash;
  bool showing_beam;
  bool showing_text;
  int flash_max_radius;
  int flash_radius;
  short flash_color;
  wchar_t flash_symbol;
  int beam_height;
  int beam_step;
  std::vector<std::pair<int, int>> text_edges;
  std::function<wchar_t(int, int)> text_show_cb;

  void init() override;
  virtual void tick(ev::timer &w, int revents) override;
};
