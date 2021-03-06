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

#include "file_backward_reader.h"
#include <string.h>
#include <wchar.h>
#include <stdexcept>
#include "config.h"
#include "file_io.h"

BackwardReader::BackwardReader(std::vector<std::vector<wchar_t>> &lines,
                               std::vector<size_t> &line_lens,
                               const Config &config)
    : lines(lines), line_lens(line_lens), config(config) {}

void BackwardReader::newPage() {
  // cache size should be equal to longest possible string
  const size_t cache_size = lines.size() * (lines[0].size() - 2) + 1;
  if (cache.size() != cache_size) {
    cache.resize(cache_size);
  }
  line_started = false;
  lines_read = 0;
  longest_line_len = 0;
  first_page = false;
  line_has_nl = false;
}

void BackwardReader::directionChanged() {
  remaining_spaces = 0;
}

bool BackwardReader::read(FileIO &f) {
  if (cache_len && processCache()) {
    return true;
  }
  while (true) {
    if (!fillCache(f)) {
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
  if ((cache[cache_end - 1] == '\n') && (cache_lines > 1)
      && ((cache_len % line_len) == 1)) {
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
    if (((start + copy_len + 1) == cache_end)
        && (cache[cache_end - 1] == '\n')) {
      ++copy_len;
    }
    size_t line_len = copy_len;
    memcpy(lines[current_line].data(), cache.data() + start,
           copy_len * mbchar_size);

    if (lines[current_line][copy_len - 1] != '\n') {
      lines[current_line][copy_len] = '\n';
      lines[current_line][copy_len + 1] = '\0';
    } else {
      --line_len;
      lines[current_line][copy_len] = '\0';
    }
    line_lens[current_line] = line_len;
    if (longest_line_len < line_len) {
      longest_line_len = line_len;
    }
    current_out_line_id = current_line;
    ++current_line;
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

bool BackwardReader::fillCache(FileIO &f) {
  if (cache_len == cache.size()) {
    return true;
  }

  while (true) {
    auto &cur_symbol = cache[cache.size() - 1 - cache_len];
    FileIO::Status ret = FileIO::Status::Ok;

    if (remaining_spaces) {
      cur_symbol = ' ';
      --remaining_spaces;
    } else {
      ret = f.read(cur_symbol);
    }
    if (ret == FileIO::Status::WouldBlock) {
      return false;
    }
    if (ret == FileIO::Status::End) {
      first_page = true;
      return true;
    }

    if (cur_symbol == '\n') {
      if (line_started) {
        f.unread();
        line_started = false;
        return true;
      }
      line_has_nl = true;
    }
    ++cache_len;
    line_started = true;
    if (cur_symbol == '\t') {
      cur_symbol = ' ';
      remaining_spaces = config.tab_width - 1;
    }
    if ((!line_has_nl && (cache_len == (cache.size() - 1)))
        || (cache_len == cache.size())) {
      return true;
    }
  }
}

size_t BackwardReader::linesRead() const {
  return lines_read;
}

wchar_t BackwardReader::get(size_t _column, size_t _row) const {
  const size_t height = lines.size();
  const size_t width = lines[0].size() - 2;
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

  if ((lines[row][column] == '\n') || (lines[row][column] == '\r')) {
    return L' ';
  }

  return lines[row][column];
}

std::wstring BackwardReader::getLine() const {
  if (current_out_line_id >= lines.size()) {
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
