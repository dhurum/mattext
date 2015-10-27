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
#include <string.h>
#include "file_reader.h"

FileReader::FileReader(const Config &config, const Terminal &terminal)
    : config(config), terminal(terminal) {
  reset();
}

void FileReader::reset() {
  bool resized = false;
  size_t line_len = terminal.getWidth();
  size_t lines_num = terminal.getHeight();

  if (lines.size() != lines_num) {
    lines.resize(lines_num);
    line_lens.resize(lines_num);
    resized = true;
  }
  if (resized || (lines[0].size() != line_len + 2)) {
    for (auto &line : lines) {
      line.resize(line_len + 2);  //'\0' + possibly '\n' from next line
    }
  }

  for (auto &line_len : line_lens) {
    line_len = 0;
  }

  current_line_id = 0;
  line_max_len = line_len;
  longest_line_len = 0;
  current_out_line_id = 0;
  prev_line_finished = true;
  mbchar_id = 0;
}

FileReader::Status FileReader::read(FILE *f) {
  if (current_line_id == lines.size()) {
    return FileReader::Status::Finished;
  }

  while (true) {
    switch(readLine(f)) {
      case FileReader::Status::WouldBlock:
        return FileReader::Status::WouldBlock;
      case FileReader::Status::Error:
        return FileReader::Status::Error;
      case FileReader::Status::Finished:
        break;
    }

    auto &cur_line = lines[current_line_id];
    auto &line_len = line_lens[current_line_id];

    if (!line_len) {
      return FileReader::Status::Finished;
    }

    if (cur_line[line_len - 1] == '\n') {
      --line_len;
      bool _prev_line_finished = prev_line_finished;
      prev_line_finished = true;

      if (!line_len && !_prev_line_finished) {
        size_t prev_line_id = current_line_id - 1;
        lines[prev_line_id][line_lens[prev_line_id]] = '\n';
        lines[prev_line_id][line_lens[prev_line_id] + 1] = '\0';
        continue;
      }
    } else {
      prev_line_finished = false;
    }

    if (longest_line_len < line_len) {
      longest_line_len = line_len;
    }
    ++current_line_id;

    if (current_line_id == lines.size()) {
      return FileReader::Status::Finished;
    }
  }
}

FileReader::Status FileReader::readLine(FILE *f) {
  auto &cur_symbol_id = line_lens[current_line_id];
  mbstate_t mbs;
  if (cur_symbol_id == line_max_len) {
    return FileReader::Status::Finished;
  }

  while (true) {
    auto &cur_symbol = lines[current_line_id][cur_symbol_id];
    cur_symbol = '\0';

    for (; mbchar_id < 4; ++mbchar_id) {
      errno = 0;
      mbchar_buf[mbchar_id] = fgetc(f);
      if (mbchar_buf[mbchar_id] == EOF) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
          return FileReader::Status::WouldBlock;
        }
        else if (errno) {
          return FileReader::Status::Error;
        }

        mbchar_id = 0;
        return FileReader::Status::Finished;
      }

      memset(&mbs, 0, sizeof(mbs));
      size_t res = mbrtowc(&cur_symbol, mbchar_buf, mbchar_id + 1, &mbs);

      if (res == (size_t)-1) {
        return FileReader::Status::Error;
      } else if (res == (size_t)-2) {
        continue;
      }
      break;
    }
    mbchar_id = 0;
    ++cur_symbol_id;
    if ((cur_symbol == '\n') || (cur_symbol_id == line_max_len)) {
      lines[current_line_id][cur_symbol_id] = '\0';
      return FileReader::Status::Finished;
    }
  }
}

size_t FileReader::linesRead() const {
  return current_line_id;
}

wchar_t FileReader::get(size_t _column, size_t _row) const {
  int start_row =
      config.center_vert ? ((terminal.getHeight() - current_line_id) / 2) : 0;
  int row = static_cast<int>(_row) - start_row;

  if ((row < 0) || (row >= static_cast<int>(current_line_id))) {
    return L' ';
  }

  size_t line_len =
      config.center_horiz_longest ? longest_line_len : line_lens[row];
  int start_column =
      config.center_horiz ? ((terminal.getWidth() - line_len) / 2) : 0;
  int column = static_cast<int>(_column) - start_column;

  if ((column < 0) || (column >= static_cast<int>(line_lens[row]))) {
    return L' ';
  }

  if (lines[row][column] == '\n') {
    return L' ';
  }

  return lines[row][column];
}

const wchar_t *FileReader::getLine() const {
  if (current_out_line_id >= current_line_id) {
    return nullptr;
  }
  return lines[current_out_line_id++].data();
}
