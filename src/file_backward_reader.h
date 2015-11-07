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
#include "file_reader_logic.h"
#include "config.h"

class BackwardReader : public FileReaderLogic {
 public:
  BackwardReader(std::vector<std::vector<wchar_t>> &lines,
                 std::vector<size_t> &line_lens, const Config &config);
  void reset() override;
  bool read(FileIO &f) override;
  size_t linesRead() const override;
  wchar_t get(size_t column, size_t row) const override;
  const wchar_t *getLine() const override;

 private:
  std::vector<std::vector<wchar_t>> &lines;
  std::vector<size_t> &line_lens;
  const Config &config;
  std::vector<wchar_t> cache;
  size_t cache_len = 0;
  size_t cache_start;
  bool line_started;
  size_t lines_read;
  mutable size_t current_out_line_id;
  size_t longest_line_len;
  bool first_page;

  bool readLine(FileIO &f);
  bool processCache();
};
