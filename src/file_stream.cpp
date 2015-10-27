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

FileStream::FileStream(const Config &config, const Terminal &terminal)
    : config(config), terminal(terminal), file_reader(config, terminal) {
  for (auto name : config.files) {
    FILE *file = fopen(name, "r");
    if (!file) {
      std::ostringstream err;
      err << "Can't open file '" << name << "'";
      throw std::runtime_error(err.str());
    }
    addFile(file, name);
  }

  if (!files.size()) {
    if (isatty(fileno(stdin))) {
      throw std::runtime_error(
          "Please, specify input files or pipe something "
          "to program input");
    }
    addFile(stdin, "stdin");
  }

  current_file = files.begin();
}

FileStream::~FileStream() {
  stop();
  for (auto &file : files) {
    fclose(file.first);
  }
}

void FileStream::addFile(FILE *file, const char *name) {
  int flags = fcntl(fileno(file), F_GETFL, 0);
  fcntl(fileno(file), F_SETFL, flags | O_NONBLOCK);
  files.push_back(std::make_pair(file, name));
}

bool FileStream::tryRewind() {
  if (!config.infinite) {
    if (on_end) {
      on_end();
    }
    return false;
  }
  for (auto &file : files) {
    if (fseek(file.first, 0, SEEK_SET)) {
      std::ostringstream err;
      err << "Can't rewind file '" << file.second
          << "' for 'infinite' option: " << strerror(errno);
      throw std::runtime_error(err.str());
    }
  }
  current_file = files.begin();
  return true;
}

void FileStream::readCb(ev::io &w, int revents) {
  //TODO: throw errors from read, here catch them and prepend with filename
  switch (file_reader.read((*current_file).first)) {
    case FileReader::Status::Finished:
      io_watcher.stop();
      if (file_reader.linesRead()) {
        on_read(file_reader);
      } else {
        ++current_file;
        if ((current_file != files.end()) || tryRewind()) {
          io_watcher.start(fileno((*current_file).first), ev::READ);
        }
      }
      break;
    case FileReader::Status::WouldBlock: {
      size_t block_lines =
          (config.block_lines < 0) ? terminal.getHeight() : config.block_lines;
      if (file_reader.linesRead() >= block_lines) {
        io_watcher.stop();
        on_read(file_reader);
      }
      break;
    }
    case FileReader::Status::Error:
      io_watcher.stop();
      std::ostringstream err;
      err << "Can't read file '" << (*current_file).second
          << "': " << strerror(errno);
      throw std::runtime_error(err.str());
  }
}

void FileStream::stop() {
  io_watcher.stop();
}

void FileStream::read(std::function<void(const Text &text)> _on_read,
                      std::function<void()> _on_end, size_t line_len,
                      size_t lines_num) {
  if (!line_len || !lines_num) {
    return;
  }
  if ((current_file == files.end()) && !tryRewind()) {
    return;
  }

  on_read = _on_read;
  on_end = _on_end;
  file_reader.reset(line_len, lines_num);

  io_watcher.set<FileStream, &FileStream::readCb>(this);
  io_watcher.start(fileno((*current_file).first), ev::READ);
}
