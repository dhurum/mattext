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

#include <ev++.h>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

class Text;
class Config;
class Terminal;

class Animation {
 public:
  virtual ~Animation() = default;
  virtual void play(const Text &text, std::function<void()> on_stop) = 0;
  virtual void stop() = 0;
  virtual bool isPlaying() = 0;
};

class AnimationStore {
 public:
  AnimationStore(const Config &config, const Terminal &terminal);
  Animation *get(std::string) const;
  static std::string getNames();

 private:
  std::unordered_map<std::string, std::unique_ptr<Animation>> animations;
};
