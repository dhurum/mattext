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
#include <unordered_set>
#include "animation_generic.h"

//TODO: GenericAnimation

class FireAnimation : public GenericAnimation {
 public:
  using GenericAnimation::GenericAnimation;

 private:
  size_t tick_id;
  std::vector<size_t> fire_heights;
  std::vector<size_t> fire_end_heights;
  size_t max_fire_height;
  size_t max_fire_end_height;
  std::unordered_set<size_t> increased_cols;
  std::unordered_set<size_t> decreased_ends;

  void init() override;
  virtual void tick(ev::timer &w, int revents) override;
};
