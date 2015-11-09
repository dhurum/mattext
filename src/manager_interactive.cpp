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

#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include "manager_interactive.h"

ManagerInteractive::ManagerInteractive(const Config &config,
                                       FileStream &file_stream,
                                       const Terminal &terminal)
    : config(config),
      file_stream(file_stream),
      terminal(terminal),
      animations(config, terminal) {
  tty_fno = open("/dev/tty", O_RDONLY);
  fcntl(tty_fno, F_SETFL, fcntl(tty_fno, F_GETFL, 0) | O_NONBLOCK);

  io_watcher.set<ManagerInteractive, &ManagerInteractive::inputCb>(this);
  io_watcher.start(tty_fno, ev::READ);

  forward_animation = animations.get("matrix");
  backward_animation = animations.get("reverse_matrix");
  current_animation = forward_animation;

  getNextPage();
}

ManagerInteractive::~ManagerInteractive() {
  close(tty_fno);
}

void ManagerInteractive::inputCb(ev::io &w, int revents) {
  int cmd = 0;

  int ret = read(tty_fno, &cmd, sizeof(int));

  if (ret == -1) {
    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
      return;
    }
    std::ostringstream err;
    err << "Can't read from terminal: " << strerror(errno);
    throw std::runtime_error(err.str());
  }

  if (!ret) {
    quit();
  }

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
      getNextPage();
      break;
    case 'b':
    case 'B':
    case 'k':
    case 'K':
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
  current_animation = forward_animation;
  file_stream.read([this](const Text &text) {
    this->current_animation->play(text, [this]() { this->checkPending(); });
  }, nullptr);
}

void ManagerInteractive::getPrevPage() {
  if (current_animation->isPlaying()) {
    pending_action = Action::Prev;
    return;
  }
  current_animation = backward_animation;
  file_stream.read([this](const Text &text) {
    this->current_animation->play(text, [this]() { this->checkPending(); });
  }, nullptr, FileIO::Direction::Backward);
}

void ManagerInteractive::quit() {
  io_watcher.stop();
  file_stream.stop();
  current_animation->stop();
  io_watcher.loop.break_loop(ev::ALL);
}
