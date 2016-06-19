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

#include <stddef.h>
#include <vector>

class FileCache {
 public:
  void addForward(char byte);
  bool readForward(char &byte);
  bool readBackward(char &byte);
  void rewindToStart();
  void rewindToEnd();
  void offsetTo(int offset);

 private:
  static const size_t cache_max_len = 1000000;
  std::vector<char> cache;
  size_t start = 0;
  size_t len = 0;
  size_t cur = 0;
  enum class Reached { Start, End, Ok };
  Reached status = Reached::End;

  static inline void incCounter(size_t &counter);
  static inline void decCounter(size_t &counter);
  inline size_t getEnd();
};
