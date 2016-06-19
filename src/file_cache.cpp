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

#include "file_cache.h"
#include <stdlib.h>

void FileCache::addForward(char byte) {
  cur = (start + len) % cache_max_len;
  if (len == cache_max_len) {
    incCounter(start);
    cache[cur] = byte;
  } else {
    cache.push_back(byte);
  }
  status = Reached::End;
  if (len < cache_max_len) {
    ++len;
  }
}

bool FileCache::readForward(char &byte) {
  if ((status == Reached::End) || !len) {
    return false;
  }
  incCounter(cur);
  byte = cache[cur];
  status = Reached::Ok;

  if (cur == getEnd()) {
    status = Reached::End;
  }
  return true;
}

bool FileCache::readBackward(char &byte) {
  if ((status == Reached::Start) || !len) {
    return false;
  }
  decCounter(cur);
  byte = cache[cur];
  status = Reached::Ok;

  if (cur == start) {
    status = Reached::Start;
  }
  return true;
}

inline void FileCache::incCounter(size_t &counter) {
  ++counter;
  if (counter >= cache_max_len) {
    counter = 0;
  }
}

inline void FileCache::decCounter(size_t &counter) {
  if (counter == 0) {
    counter = cache_max_len - 1;
    return;
  }
  --counter;
}

inline size_t FileCache::getEnd() {
  return (start + len - 1) % cache_max_len;
}

void FileCache::rewindToStart() {
  if (!len) {
    return;
  }
  cur = start;
  status = Reached::Start;
}

void FileCache::rewindToEnd() {
  if (!len) {
    return;
  }
  cur = getEnd();
  status = Reached::End;
}

void FileCache::offsetTo(int _offset) {
  if (!len) {
    return;
  }
  const size_t end = getEnd();
  const size_t offset = abs(_offset);
  size_t start_fix = 0;

  if (_offset > 0) {
    if (cur >= start) {
      start_fix = start;
    }
    if ((cur - start_fix + offset) >= end) {
      cur = end;
      status = Reached::End;
    } else {
      cur = (cur + offset) % cache_max_len;
      status = Reached::Ok;
    }
    return;
  }
  if (cur < start) {
    start_fix = len;
  }
  if ((cur + start_fix - start) < offset) {
    cur = start;
    status = Reached::Start;
  } else {
    cur -= offset;
    status = Reached::Ok;
  }
}
