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
#include "animation_none.h"
#include "animation_fire.h"
#include "animation_beam.h"

struct AnimationInfo {
  std::string name;
  std::function<std::unique_ptr<Animation>(const Config &, const Terminal &)>
      make;
};

static std::list<AnimationInfo> animation_info{
    {"matrix",
     [](const Config &config, const Terminal &terminal) {
       return std::make_unique<MatrixAnimation>(config, terminal);
     }},
    {"reverse_matrix",
     [](const Config &config, const Terminal &terminal) {
       return std::make_unique<ReverseMatrixAnimation>(config, terminal);
     }},
    {"none",
     [](const Config &config, const Terminal &terminal) {
       return std::make_unique<NoneAnimation>(terminal);
     }},
    {"fire", [](const Config &config, const Terminal &terminal) {
       return std::make_unique<FireAnimation>(config, terminal);
     }},
    {"beam", [](const Config &config, const Terminal &terminal) {
       return std::make_unique<BeamAnimation>(config, terminal);
     }}};

AnimationStore::AnimationStore(const Config &config, const Terminal &terminal) {
  for (auto &info : animation_info) {
    animations[info.name] = info.make(config, terminal);
  }
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

std::string AnimationStore::getNames() {
  std::string names;

  for (auto &info : animation_info) {
    if (names.size()) {
      names += ", ";
    }
    names += info.name;
  }

  return names;
}
