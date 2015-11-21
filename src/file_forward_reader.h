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

class Config;

class ForwardReader : public FileReaderLogic {
 public:
  ForwardReader(std::vector<std::vector<wchar_t>> &lines,
                std::vector<size_t> &line_lens, const Config &config);
  void reset() override;
  bool read(FileIO &f) override;
  size_t linesRead() const override;
  wchar_t get(size_t column, size_t row) const override;
  std::wstring getLine() const override;

 private:
  std::vector<std::vector<wchar_t>> &lines;
  std::vector<size_t> &line_lens;
  const Config &config;
  size_t current_line_id;
  size_t longest_line_len;
  mutable size_t current_out_line_id;

  bool readLine(FileIO &f);
};
