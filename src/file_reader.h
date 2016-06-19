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

#include <memory>
#include <vector>
#include "direction.h"

class Terminal;
class Config;
class FileReaderLogic;
class FileIO;

class Text {
 public:
  virtual wchar_t get(size_t column, size_t row) const = 0;
  virtual std::wstring getLine() const = 0;
};

class FileReader : public Text {
 public:
  FileReader(const Config &config, const Terminal &terminal);
  ~FileReader();
  void newPage(Direction direction);
  bool read(FileIO &f);
  size_t linesRead() const;
  wchar_t get(size_t column, size_t row) const override;
  std::wstring getLine() const override;

 private:
  const Terminal &terminal;
  std::vector<std::vector<wchar_t>> lines;
  std::vector<size_t> line_lens;
  std::unique_ptr<FileReaderLogic> forward_reader;
  std::unique_ptr<FileReaderLogic> backward_reader;
  FileReaderLogic *reader = nullptr;
  Direction direction = Direction::Forward;
};
