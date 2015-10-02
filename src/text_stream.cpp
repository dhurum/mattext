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

#include <string.h>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>
#include <sstream>
#include "text_stream.h"

TextStream::TextStream(const Config &config, const Terminal &terminal)
    : config(config), terminal(terminal), text(config, terminal) {
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

TextStream::~TextStream() {
  stop();
  for (auto &file : files) {
    fclose(file.first);
  }
}

void TextStream::addFile(FILE *file, const char *name) {
  int flags = fcntl(fileno(file), F_GETFL, 0);
  fcntl(fileno(file), F_SETFL, flags | O_NONBLOCK);
  files.push_back(std::make_pair(file, name));
}

bool TextStream::tryRewind() {
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

void TextStream::readCb(ev::io &w, int revents) {
  while (true) {
    errno = 0;
    auto &cur_line = text.lines[text.lines_read];
    wchar_t *ret =
        fgetws(cur_line.data(), line_max_len + 1, (*current_file).first);

    if (!ret) {
      if (!errno) {
        io_watcher.stop();
        ++current_file;
        if (text.lines_read) {
          on_read(text);
        } else if ((current_file != files.end()) || tryRewind()) {
          io_watcher.start(fileno((*current_file).first), ev::READ);
        }
      } else if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        size_t block_lines =
            (config.block_lines < 0) ? terminal.getWidth() : config.block_lines;
        if (text.lines_read >= block_lines) {
          io_watcher.stop();
          text.lines.resize(text.lines_read);
          on_read(text);
        }
      } else {
        std::ostringstream err;
        err << "Can't read file '" << (*current_file).second
            << "': " << strerror(errno);
        throw std::runtime_error(err.str());
      }
      return;
    }

    size_t line_len = wcslen(cur_line.data());
    text.lines_len[text.lines_read] = line_len;
    if (cur_line.data()[line_len - 1] == '\n') {
      --line_len;
      bool _prev_line_finished = prev_line_finished;
      prev_line_finished = true;

      if (!line_len && !_prev_line_finished) {
        size_t prev_line_id = text.lines_read - 1;
        text.lines[prev_line_id][text.lines_len[prev_line_id]] = '\n';
        text.lines[prev_line_id][text.lines_len[prev_line_id] + 1] = '\0';
        continue;
      }
    } else {
      prev_line_finished = false;
    }
    if (text.longest_line_len < line_len) {
      text.longest_line_len = line_len;
    }
    ++text.lines_read;

    if (text.lines_read == lines_num) {
      io_watcher.stop();
      on_read(text);
      return;
    }
  }
}

void TextStream::stop() {
  io_watcher.stop();
}

void TextStream::read(std::function<void(const Text &text)> _on_read,
                      std::function<void()> _on_end, size_t _line_max_len,
                      size_t _lines_num) {
  if (!_line_max_len || !_lines_num) {
    return;
  }
  if ((current_file == files.end()) && !tryRewind()) {
    return;
  }
  line_max_len = _line_max_len;
  lines_num = _lines_num;
  on_read = _on_read;
  on_end = _on_end;

  text.reset(line_max_len, lines_num);
  prev_line_finished = true;

  io_watcher.set<TextStream, &TextStream::readCb>(this);
  io_watcher.start(fileno((*current_file).first), ev::READ);
}

Text::Text(const Config &config, const Terminal &terminal)
    : config(config), terminal(terminal) {}

void Text::reset(size_t line_max_len, size_t lines_num) {
  bool resized = false;

  if (lines.size() != lines_num) {
    lines.resize(lines_num);
    lines_len.resize(lines_num);
    resized = true;
  }
  if (resized || (lines[0].size() != line_max_len + 1)) {
    for (auto &line : lines) {
      line.resize(line_max_len + 2);  //'\0' + possibly '\n' from next line
    }
  }
  lines_read = 0;
  longest_line_len = 0;
  cur_out_line = 0;
}

wchar_t Text::get(size_t _column, size_t _row) const noexcept {
  int start_row =
      config.center_vert ? ((terminal.getHeight() - lines_read) / 2) : 0;
  int row = static_cast<int>(_row) - start_row;

  if ((row < 0) || (row >= static_cast<int>(lines_read))) {
    return L' ';
  }

  size_t line_len =
      config.center_horiz_longest ? longest_line_len : lines_len[row];
  int start_column =
      config.center_horiz ? ((terminal.getWidth() - line_len) / 2) : 0;
  int column = static_cast<int>(_column) - start_column;

  if ((column < 0) || (column >= static_cast<int>(lines_len[row]))) {
    return L' ';
  }

  if (lines[row][column] == '\n') {
    return L' ';
  }

  return lines[row][column];
}

const wchar_t *Text::getLine() const noexcept {
  if (cur_out_line >= lines_read) {
    return nullptr;
  }
  return lines[cur_out_line++].data();
}
