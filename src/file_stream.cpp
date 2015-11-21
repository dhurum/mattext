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
#include <stdexcept>
#include <unistd.h>
#include <sstream>
#include "file_stream.h"
#include "config.h"
#include "terminal.h"

FileStream::FileStream(const Config &config, const Terminal &terminal)
    : config(config), terminal(terminal), file_reader(config, terminal) {
  for (auto name : config.files) {
    files.push_back(std::make_unique<FileIO>(name));
  }

  if (!files.size()) {
    if (terminal.stdinIsTty()) {
      throw std::runtime_error(
          "Please, specify input files or pipe something "
          "to program input");
    }
    files.push_back(std::make_unique<FileIO>(terminal.stdinFd()));
  }

  current_file = files.begin();
}

FileStream::~FileStream() {
  stop();
}

bool FileStream::nextFile() {
  auto prev_file = current_file;

  if (direction == FileIO::Direction::Forward) {
    ++current_file;
    if (current_file != files.end()) {
      (**prev_file).stop();
      (**current_file).newPage(direction);
      return true;
    }
  } else if (current_file != files.begin()) {
    --current_file;
    (**prev_file).stop();
    (**current_file).newPage(direction);
    return true;
  }
  if (!config.infinite) {
    end_reached = true;
    return false;
  }

  if (direction == FileIO::Direction::Forward) {
    current_file = files.begin();
  } else {
    current_file = files.end();
    --current_file;
  }

  (**prev_file).stop();
  (**current_file).newPage(direction);
  return true;
}

void FileStream::readCb(ev::io &w, int revents) {
  if (!file_reader.read(**current_file)) {
    size_t block_lines =
        (config.block_lines < 0) ? terminal.getHeight() : config.block_lines;
    if (file_reader.linesRead() >= block_lines) {
      io_watcher.stop();
      on_read(file_reader);
    }
    return;
  }

  io_watcher.stop();
  if (file_reader.linesRead()) {
    on_read(file_reader);
  } else if (nextFile()) {
    io_watcher.start((**current_file).fno(), ev::READ);
  } else if (on_end) {
    on_end();
  }
}

void FileStream::stop() {
  io_watcher.stop();
}

void FileStream::read(std::function<void(const Text &text)> _on_read,
                      std::function<void()> _on_end,
                      FileIO::Direction _direction) {
  if (end_reached) {
    if (_direction == direction) {
      return;
    } else if (_direction == FileIO::Direction::Backward) {
      --current_file;
    }
  }

  end_reached = false;
  on_read = _on_read;
  on_end = _on_end;
  direction = _direction;

  (**current_file).newPage(direction);
  file_reader.reset(direction);

  io_watcher.set<FileStream, &FileStream::readCb>(this);
  io_watcher.start((**current_file).fno(), ev::READ);
}
