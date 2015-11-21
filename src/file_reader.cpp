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

#include "file_reader.h"
#include "file_forward_reader.h"
#include "file_backward_reader.h"
#include "config.h"
#include "terminal.h"

FileReader::FileReader(const Config &config, const Terminal &terminal)
    : terminal(terminal),
      forward_reader(std::make_unique<ForwardReader>(lines, line_lens, config)),
      backward_reader(
          std::make_unique<BackwardReader>(lines, line_lens, config)) {}

void FileReader::reset(FileIO::Direction direction) {
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

  if (direction == FileIO::Direction::Forward) {
    reader = forward_reader.get();
  } else {
    reader = backward_reader.get();
  }
  reader->reset();
}

bool FileReader::read(FileIO &f) {
  return reader->read(f);
}

size_t FileReader::linesRead() const {
  return reader->linesRead();
}

wchar_t FileReader::get(size_t column, size_t row) const {
  return reader->get(column, row);
}

std::wstring FileReader::getLine() const {
  return reader->getLine();
}
