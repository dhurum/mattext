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

static const size_t mbchar_size = 4;

class FileIO {
 public:
  enum class Direction { Forward, Backward };
  enum class Status { Ok, End, WouldBlock };
  FileIO(const char *name);
  FileIO(int stdin_fd);
  ~FileIO();
  void stop();
  void newPage(Direction direction);
  Status read(wchar_t &symbol);
  void unread();
  int fno();

 private:
  int fd;
  const char *name;
  bool is_pipe = false;
  char mbchar_buf[mbchar_size];
  size_t mbchar_id = 0;
  Direction direction = Direction::Forward;
  size_t bytes_read = 0;
  size_t prev_bytes_read = 0;
  bool started = false;
  bool active = false;

  Status readForward(wchar_t &symbol);
  Status readBackward(wchar_t &symbol);
};
