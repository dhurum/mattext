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

#include <string>

class FileIO;

class FileReaderLogic {
 public:
  virtual ~FileReaderLogic() = default;
  virtual void newPage() = 0;
  virtual void directionChanged() = 0;
  virtual bool read(FileIO &f) = 0;
  virtual size_t linesRead() const = 0;
  virtual wchar_t get(size_t column, size_t row) const = 0;
  virtual std::wstring getLine() const = 0;
};
