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

class MatrixAnimation : public GenericAnimation {
 public:
  using GenericAnimation::GenericAnimation;

 protected:
  std::vector<size_t> col_lengths;
  std::vector<size_t> col_offsets;
  size_t max_col_length;
  size_t tick_id;
  size_t tail_length = 10;

  void init() override;
  virtual void tick(ev::timer &w, int revents) override;
  wchar_t getRandSymbol();
};
