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

#include <sstream>
#include <stdexcept>
#include <string.h>
#include <ev++.h>
#include <ncurses.h>
#include "manager_interactive.h"

ManagerInteractive::ManagerInteractive(const Config &config,
                                       FileStream &file_stream,
                                       const Terminal &terminal)
    : config(config),
      file_stream(file_stream),
      terminal(terminal),
      animations(config, terminal) {
  terminal.onKeyPress([this](int cmd) { this->inputCb(cmd); });

  animation_next = animations.get(config.animation_next);
  animation_prev = animations.get(config.animation_prev);
  current_animation = animation_next;

  getNextPage();
}

void ManagerInteractive::inputCb(int cmd) {
  switch (cmd) {
    case 'q':
    case 'Q':
    case '\4':
      quit();
      break;
    case 'f':
    case 'F':
    case 'j':
    case 'J':
    case ' ':
    case KEY_NPAGE:
    case KEY_DOWN:
      getNextPage();
      break;
    case 'b':
    case 'B':
    case 'k':
    case 'K':
    case KEY_PPAGE:
    case KEY_UP:
      getPrevPage();
      break;
    default:
      break;
  }
}

void ManagerInteractive::checkPending() {
  auto action = pending_action;
  pending_action = Action::None;
  if ((action == Action::Next) || config.noninteract) {
    getNextPage();
  } else if (action == Action::Prev) {
    getPrevPage();
  }
}

void ManagerInteractive::getNextPage() {
  if (current_animation->isPlaying()) {
    pending_action = Action::Next;
    return;
  }
  current_animation = animation_next;
  file_stream.read([this](const Text &text) {
    this->current_animation->play(text, [this]() { this->checkPending(); });
  }, nullptr);
}

void ManagerInteractive::getPrevPage() {
  if (current_animation->isPlaying()) {
    pending_action = Action::Prev;
    return;
  }
  current_animation = animation_prev;
  file_stream.read([this](const Text &text) {
    this->current_animation->play(text, [this]() { this->checkPending(); });
  }, nullptr, FileIO::Direction::Backward);
}

void ManagerInteractive::quit() {
  file_stream.stop();
  current_animation->stop();
  terminal.stop();
  ev::get_default_loop().break_loop(ev::ALL);
}
