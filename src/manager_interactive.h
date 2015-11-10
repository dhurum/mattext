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

#include "config.h"
#include "file_stream.h"
#include "terminal.h"
#include "animation.h"
#include "manager.h"

class ManagerInteractive : public Manager {
 public:
  ManagerInteractive(const Config &config, FileStream &file_stream,
                     const Terminal &terminal);
  void checkPending();

 private:
  const Config &config;
  FileStream &file_stream;
  const Terminal &terminal;
  AnimationStore animations;
  Animation *animation_next;
  Animation *animation_prev;
  Animation *current_animation;
  enum class Action { None, Next, Prev };
  Action pending_action = Action::None;

  void getNextPage();
  void getPrevPage();
  void inputCb(int cmd);
  void quit();
};
