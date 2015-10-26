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

#include <vector>
#include "config.h"
#include "terminal.h"

class Text {
 public:
  virtual wchar_t get(size_t column, size_t row) const = 0;
  virtual const wchar_t *getLine() const = 0;
};

class FileReader : public Text {
 public:
  enum class Status { Finished, WouldBlock, Error };
  FileReader(const Config &config, const Terminal &terminal);
  void reset(size_t line_len, size_t lines_num);
  FileReader::Status read(FILE *f);
  size_t linesRead() const;
  wchar_t get(size_t column, size_t row) const override;
  const wchar_t *getLine() const override;

 private:
  const Config &config;
  const Terminal &terminal;
  std::vector<std::vector<wchar_t>> lines;
  std::vector<size_t> line_lens;
  size_t current_line_id;
  size_t line_max_len;
  size_t longest_line_len;
  mutable size_t current_out_line_id;
  bool prev_line_finished;
};
