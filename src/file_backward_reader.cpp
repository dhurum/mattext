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
#include "file_backward_reader.h"

BackwardReader::BackwardReader(std::vector<std::vector<wchar_t>> &lines,
                               std::vector<size_t> &line_lens,
                               const Config &config)
    : lines(lines), line_lens(line_lens), config(config) {}

void BackwardReader::reset() {
  // cache size should be equal to longest possible string
  const size_t cache_size = lines.size() * (lines[0].size() - 1);
  if (cache.size() != cache_size) {
    cache.resize(cache_size);
  }
  line_started = false;
  lines_read = 0;
  longest_line_len = 0;
  first_page = false;
}

bool BackwardReader::read(FileIO &f) {
  if (cache_len && processCache()) {
    return true;
  }
  while (true) {
    if (!readLine(f)) {
      return false;
    }
    if (!cache_len) {
      return true;
    }
    cache_start = cache.size() - cache_len;
    if (processCache()) {
      return true;
    }
  }
}

bool BackwardReader::processCache() {
  size_t lines_remaining = lines.size() - lines_read;
  if (!lines_remaining) {
    return true;
  }
  const size_t line_len = lines[0].size() - 2;
  const size_t cache_end = cache_start + cache_len;
  size_t cache_lines = cache_len / line_len + (cache_len % line_len ? 1 : 0);
  if ((cache[cache_end - 1] == '\n') && (cache_lines > 1) &&
      ((cache_len % line_len) == 1)) {
    --cache_lines;
  }
  size_t start = cache_start;
  if (cache_lines > lines_remaining) {
    start += line_len * (cache_lines - lines_remaining);
    cache_lines = lines_remaining;
  }
  size_t current_line = lines_remaining - cache_lines;

  while (cache_lines) {
    size_t copy_len =
        ((start + line_len) > cache_end) ? cache_end - start : line_len;
    if (((start + copy_len + 1) == cache_end) &&
        (cache[cache_end - 1] == '\n')) {
      ++copy_len;
    }
    memcpy(lines[current_line].data(), cache.data() + start,
           copy_len * mbchar_size);
    line_lens[current_line] = copy_len;
    lines[current_line][copy_len] = '\0';
    if (longest_line_len < copy_len) {
      longest_line_len = copy_len;
    }
    current_out_line_id = current_line;
    --current_line;
    --lines_remaining;
    --cache_lines;
    ++lines_read;
    start += copy_len;
    cache_len -= copy_len;
  }

  if (!lines_remaining) {
    return true;
  }
  return false;
}

bool BackwardReader::readLine(FileIO &f) {
  if (cache_len == cache.size()) {
    return true;
  }

  while (true) {
    auto &cur_symbol = cache[cache.size() - 1 - cache_len];
    FileIO::Status ret = f.read(cur_symbol);
    if (ret == FileIO::Status::WouldBlock) {
      return false;
    } else if (ret == FileIO::Status::End) {
      first_page = true;
      return true;
    }

    if ((cur_symbol == '\n') && line_started) {
      f.unread();
      line_started = false;
      return true;
    }
    ++cache_len;
    line_started = true;
    if (cache_len == cache.size()) {
      return true;
    }
  }
}

size_t BackwardReader::linesRead() const {
  return lines_read;
}

wchar_t BackwardReader::get(size_t _column, size_t _row) const {
  const size_t height = lines.size();
  const size_t width = lines[0].size();
  const int first_line = lines.size() - lines_read;
  int start_row = -first_line;
  if (config.center_vert) {
    start_row += ((height - lines_read) / 2);
  } else if (first_page) {
    start_row = 0;
  }
  const int row = static_cast<int>(_row) - start_row;

  if ((row < first_line) || (row >= static_cast<int>(lines.size()))) {
    return L' ';
  }

  size_t line_len =
      config.center_horiz_longest ? longest_line_len : line_lens[row];
  int start_column = config.center_horiz ? ((width - line_len) / 2) : 0;
  int column = static_cast<int>(_column) - start_column;

  if ((column < 0) || (column >= static_cast<int>(line_lens[row]))) {
    return L' ';
  }

  if (lines[row][column] == '\n') {
    return L' ';
  }

  return lines[row][column];
}

const wchar_t *BackwardReader::getLine() const {
  if (current_out_line_id >= lines.size()) {
    return nullptr;
  }
  return lines[current_out_line_id++].data();
}
