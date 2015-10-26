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

#include <list>
#include <functional>
#include <ev++.h>
#include "config.h"
#include "terminal.h"
#include "file_reader.h"

class FileStream {
 public:
  FileStream(const Config &config, const Terminal &terminal);
  ~FileStream();
  void stop();
  void read(std::function<void(const Text &text)> on_read,
            std::function<void()> on_end, size_t line_max_len,
            size_t lines_num);

 private:
  const Config &config;
  const Terminal &terminal;
  FileReader file_reader;
  std::list<std::pair<FILE *, const char *>> files;
  std::list<std::pair<FILE *, const char *>>::iterator current_file;
  ev::io io_watcher;
  std::function<void(const Text &text)> on_read;
  std::function<void()> on_end;

  void readCb(ev::io &w, int revents);
  void addFile(FILE *file, const char *name);
  bool tryRewind();
};
