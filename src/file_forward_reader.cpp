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
#include <string.h>
#include "file_forward_reader.h"
#include "config.h"
#include "file_io.h"

ForwardReader::ForwardReader(std::vector<std::vector<wchar_t>> &lines,
                             std::vector<size_t> &line_lens,
                             const Config &config)
    : lines(lines), line_lens(line_lens), config(config) {}

void ForwardReader::reset() {
  current_line_id = 0;
  longest_line_len = 0;
  current_out_line_id = 0;
}

bool ForwardReader::read(FileIO &f) {
  if (current_line_id == lines.size()) {
    return true;
  }

  while (true) {
    if (!readLine(f)) {
      return false;
    }

    auto &cur_line = lines[current_line_id];
    auto &line_len = line_lens[current_line_id];

    if (!line_len) {
      return true;
    }

    if (cur_line[line_len - 1] == '\n') {
      --line_len;
    }

    if (longest_line_len < line_len) {
      longest_line_len = line_len;
    }
    ++current_line_id;

    if (current_line_id == lines.size()) {
      return true;
    }
  }
}

bool ForwardReader::readLine(FileIO &f) {
  auto &cur_symbol_id = line_lens[current_line_id];
  const size_t line_max_len = lines[0].size() - 1;
  if (cur_symbol_id == line_max_len) {
    return true;
  }

  while (true) {
    auto &cur_symbol = lines[current_line_id][cur_symbol_id];
    FileIO::Status ret = f.read(cur_symbol);
    if (ret == FileIO::Status::WouldBlock) {
      return false;
    } else if (ret == FileIO::Status::End) {
      return true;
    }
    ++cur_symbol_id;

    if ((cur_symbol != '\n') && (cur_symbol_id < line_max_len)) {
      continue;
    }

    if (cur_symbol != '\n') {
      f.unread();
      cur_symbol = '\n';
    }
    lines[current_line_id][cur_symbol_id] = '\0';
    return true;
  }
}

size_t ForwardReader::linesRead() const {
  return current_line_id;
}

wchar_t ForwardReader::get(size_t _column, size_t _row) const {
  const size_t height = lines.size();
  const size_t width = lines[0].size() - 2;
  int start_row = config.center_vert ? ((height - current_line_id) / 2) : 0;
  int row = static_cast<int>(_row) - start_row;

  if ((row < 0) || (row >= static_cast<int>(current_line_id))) {
    return L' ';
  }

  size_t line_len =
      config.center_horiz_longest ? longest_line_len : line_lens[row];
  int start_column = config.center_horiz ? ((width - line_len) / 2) : 0;
  int column = static_cast<int>(_column) - start_column;

  if ((column < 0) || (column >= static_cast<int>(line_lens[row]))) {
    return L' ';
  }

  if ((lines[row][column] == '\n') || (lines[row][column] == '\r')) {
    return L' ';
  }

  return lines[row][column];
}

std::wstring ForwardReader::getLine() const {
  if (current_out_line_id >= current_line_id) {
    return L"";
  }
  if (!config.center_horiz) {
    return lines[current_out_line_id++].data();
  }

  const size_t width = lines[0].size() - 2;
  size_t line_len = config.center_horiz_longest
                        ? longest_line_len
                        : line_lens[current_out_line_id];
  size_t padding = (width - line_len) / 2;
  std::wstring str;
  if (padding) {
    str.append(padding, L' ');
  }

  str.append(lines[current_out_line_id].data());
  ++current_out_line_id;

  return str;
}
