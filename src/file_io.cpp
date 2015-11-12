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

#include <fcntl.h>
#include <stdexcept>
#include <string.h>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include "file_io.h"

FileIO::FileIO(const char *name) : name(name) {
  fd = open(name, O_RDONLY | O_NONBLOCK);
  if (fd == -1) {
    std::ostringstream err;
    err << "Can't open file '" << name << "': " << strerror(errno);
    throw std::runtime_error(err.str());
  }

  struct stat file_stat;
  if (fstat(fd, &file_stat)) {
    std::ostringstream err;
    err << "Can't get file '" << name << "' stat: " << strerror(errno);
    throw std::runtime_error(err.str());
  }
  if (S_ISFIFO(file_stat.st_mode)) {
    is_pipe = true;
  }
}

FileIO::FileIO(int stdin_fd) : fd(stdin_fd), name("stdin"), is_pipe(true) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    std::ostringstream err;
    err << "Can't get flags of stdin: " << strerror(errno);
    throw std::runtime_error(err.str());
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    std::ostringstream err;
    err << "Can't set flags for stdin: " << strerror(errno);
    throw std::runtime_error(err.str());
  }
}

FileIO::~FileIO() {
  close(fd);
}

FileIO::Status FileIO::readForward(wchar_t &symbol) {
  mbstate_t mbs;
  symbol = '\0';
  bool symbol_decoded = false;

  for (; mbchar_id < mbchar_size; ++mbchar_id) {
    errno = 0;
    int ret = ::read(fd, mbchar_buf + mbchar_id, 1);

    if (!ret) {
      if (mbchar_id) {
        mbchar_id = 0;
        symbol = L'\ufffd';
        return Status::Ok;
      }
      return Status::End;
    }
    if (ret == -1) {
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        return Status::WouldBlock;
      }
      std::ostringstream err;
      err << "Can't read from file '" << name << "': " << strerror(errno);
      throw std::runtime_error(err.str());
    }
    ++bytes_read;

    memset(&mbs, 0, sizeof(mbs));
    size_t res = mbrtowc(&symbol, mbchar_buf, mbchar_id + 1, &mbs);

    if (res == static_cast<size_t>(-2)) {
      continue;
    } else if (res != static_cast<size_t>(-1)) {
      symbol_decoded = true;
    }
    break;
  }
  if (!symbol_decoded) {
    symbol = L'\ufffd';
  }
  mbchar_id = 0;
  return Status::Ok;
}

FileIO::Status FileIO::readBackward(wchar_t &symbol) {
  mbstate_t mbs;
  symbol = '\0';
  bool symbol_decoded = false;

  for (; mbchar_id < mbchar_size; ++mbchar_id) {
    size_t mbchar_cur_id = mbchar_size - 1 - mbchar_id;

    if (lseek(fd, -1, SEEK_CUR) == -1) {
      int seek_errno = errno;
      if (lseek(fd, 0, SEEK_CUR)) {
        std::ostringstream err;
        err << "Can't read from file '" << name
            << "': seek failed: " << strerror(seek_errno);
        throw std::runtime_error(err.str());
      }
      if (mbchar_id) {
        mbchar_id = 0;
        symbol = L'\ufffd';
        return Status::Ok;
      }
      return Status::End;
    }
    errno = 0;
    int ret = ::read(fd, mbchar_buf + mbchar_cur_id, 1);

    if (ret <= 0) {
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        return Status::WouldBlock;
      }
      std::ostringstream err;
      err << "Can't read from file '" << name << "': " << strerror(errno);
      throw std::runtime_error(err.str());
    }
    if (lseek(fd, -1, SEEK_CUR) == -1) {
      std::ostringstream err;
      err << "Can't read from file '" << name
          << "': seek failed: " << strerror(errno);
      throw std::runtime_error(err.str());
    }
    ++bytes_read;

    memset(&mbs, 0, sizeof(mbs));
    size_t res =
        mbrtowc(&symbol, mbchar_buf + mbchar_cur_id, mbchar_id + 1, &mbs);
    if ((res != static_cast<size_t>(-1)) && (res != static_cast<size_t>(-2))) {
      symbol_decoded = true;
      break;
    }
  }
  if (!symbol_decoded) {
    symbol = L'\ufffd';
  }
  mbchar_id = 0;
  return Status::Ok;
}

void FileIO::stop() {
  active = false;
}

void FileIO::newPage(Direction _direction) {
  if (!active && (started || (_direction == Direction::Backward)) && !is_pipe) {
    int ret = 0;
    if (_direction == Direction::Forward) {
      ret = lseek(fd, 0, SEEK_SET);
    } else {
      ret = lseek(fd, 0, SEEK_END);
    }
    if (ret == -1) {
      std::ostringstream err;
      err << "Can' seek in file '" << name << "': " << strerror(errno);
      throw std::runtime_error(err.str());
    }
  }
  if (active && (direction != _direction) && !is_pipe) {
    if (!bytes_read && prev_bytes_read) {
      bytes_read = prev_bytes_read;
    }
    int offset = bytes_read;
    if (direction == Direction::Forward) {
      offset *= -1;
    }
    if (offset && (lseek(fd, offset, SEEK_CUR) == -1)) {
      std::ostringstream err;
      err << "Can' seek in file '" << name << "': " << strerror(errno);
      throw std::runtime_error(err.str());
    }
  }
  direction = _direction;
  active = true;
  started = true;
  prev_bytes_read = bytes_read;
  bytes_read = 0;
}

FileIO::Status FileIO::read(wchar_t &symbol) {
  if (direction == Direction::Forward) {
    return readForward(symbol);
  }
  if (is_pipe) {
    return Status::End;
  }
  return readBackward(symbol);
}

void FileIO::unread() {
  int offset = 1;
  if (direction == Direction::Forward) {
    offset = -1;
  }
  if (lseek(fd, offset, SEEK_CUR) == -1) {
    std::ostringstream err;
    err << "Can't unread from file '" << name
        << "': seek failed: " << strerror(errno);
    throw std::runtime_error(err.str());
  }
  --bytes_read;
}

int FileIO::fno() {
  return fd;
}
