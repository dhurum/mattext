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

#include <stdexcept>
#include <sstream>
#include "animation.h"
#include "animation_matrix.h"
#include "animation_reverse_matrix.h"

AnimationStore::AnimationStore(const Config &config, const Terminal &terminal)
    : config(config), terminal(terminal) {
  animations["matrix"] = std::make_unique<MatrixAnimation>(config, terminal);
  animations["reverse_matrix"] =
      std::make_unique<ReverseMatrixAnimation>(config, terminal);
}

Animation *AnimationStore::get(std::string name) const {
  try {
    return animations.at(name).get();
  } catch (std::out_of_range &e) {
    std::ostringstream err;
    err << "Unknown animation '" << name << "'";
    throw std::runtime_error(err.str());
  }
}
