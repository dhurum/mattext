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
#include "file_reader.h"
#include "file_io.h"

class Terminal;
class Config;

class FileStream {
 public:
  FileStream(const Config &config, const Terminal &terminal);
  ~FileStream();
  void stop();
  void read(std::function<void(const Text &text)> on_read,
            std::function<void()> on_end,
            FileIO::Direction direction = FileIO::Direction::Forward);

 private:
  const Config &config;
  const Terminal &terminal;
  FileReader file_reader;
  using FileList = std::list<std::unique_ptr<FileIO>>;
  FileList files;
  FileList::iterator current_file;
  ev::io io_watcher;
  std::function<void(const Text &text)> on_read;
  std::function<void()> on_end;
  FileIO::Direction direction;
  bool end_reached = false;
  size_t block_lines;

  void readCb(ev::io &w, int revents);
  bool nextFile();
  void switchDirection();
};
