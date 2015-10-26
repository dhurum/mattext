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

#include <wchar.h>
#include <stdexcept>
#include <sstream>
#include <ev++.h>
#include <string.h>
#include "manager_plain.h"

ManagerPlain::ManagerPlain(const Config &config, FileStream &file_stream,
                           const Terminal &terminal)
    : config(config), file_stream(file_stream), terminal(terminal) {
  read();
}

ManagerPlain::~ManagerPlain() {}

void ManagerPlain::read() {
  file_stream.read(
      [this](const Text &text) {
        while (true) {
          const wchar_t *line = text.getLine();
          if (!line) {
            this->read();
            return;
          }
          if (fputws(line, stdout) == -1) {
            std::ostringstream err;
            err << "Can't write to stdout: " << strerror(errno);
            throw std::runtime_error(err.str());
          }
        }
      },
      [this]() { this->quit(); }, terminal.getWidth(), terminal.getHeight());
}

void ManagerPlain::quit() {
  ev::get_default_loop().break_loop(ev::ALL);
}
