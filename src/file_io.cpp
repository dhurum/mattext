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
#include "file_cache.h"

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
    cache = std::make_unique<FileCache>();
  }
}

FileIO::FileIO(int stdin_fd)
    : fd(stdin_fd),
      name("stdin"),
      cache(std::make_unique<FileCache>()) {
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

FileIO::Status FileIO::readByteForward(char *byte_ptr) {
  if (cache && cache->readForward(*byte_ptr)) {
    return Status::Ok;
  }
  int ret = ::read(fd, byte_ptr, 1);

  if (!ret) {
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
  if (cache) {
    cache->addForward(*byte_ptr);
  }
  return Status::Ok;
}

FileIO::Status FileIO::readForward(wchar_t &symbol) {
  mbstate_t mbs;
  symbol = '\0';
  bool symbol_decoded = false;

  for (; mbchar_id < mbchar_size; ++mbchar_id) {
    errno = 0;
    const Status read_res = readByteForward(mbchar_buf + mbchar_id);
    if (read_res != Status::Ok) {
      if ((read_res == Status::End) && mbchar_id) {
        mbchar_id = 0;
        symbol = L'\ufffd';
        return Status::Ok;
      }
      return read_res;
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
  
FileIO::Status FileIO::readByteBackward(char *byte_ptr) {
  if (cache && cache->readBackward(*byte_ptr)) {
    return Status::Ok;
  } else {
    return Status::End;
  }

  if (lseek(fd, -1, SEEK_CUR) == -1) {
    int seek_errno = errno;
    if (lseek(fd, 0, SEEK_CUR)) {
      std::ostringstream err;
      err << "Can't read from file '" << name
        << "': seek failed: " << strerror(seek_errno);
      throw std::runtime_error(err.str());
    }
    return Status::End;
  }
  errno = 0;
  int ret = ::read(fd, byte_ptr, 1);

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
  return Status::Ok;
}

FileIO::Status FileIO::readBackward(wchar_t &symbol) {
  mbstate_t mbs;
  symbol = '\0';
  bool symbol_decoded = false;

  for (; mbchar_id < mbchar_size; ++mbchar_id) {
    const size_t mbchar_cur_id = mbchar_size - 1 - mbchar_id;
    const Status read_res = readByteBackward(mbchar_buf + mbchar_cur_id);

    if (read_res != Status::Ok) {
      if ((read_res == Status::End) && mbchar_id) {
        mbchar_id = 0;
        symbol = L'\ufffd';
        return Status::Ok;
      }
      return read_res;
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
  if (!active && (started || (_direction == Direction::Backward))) {
    int ret = 0;
    if (_direction == Direction::Forward) {
      if (cache) {
        cache->rewindToStart();
      } else {
        ret = lseek(fd, 0, SEEK_SET);
      }
    } else {
      if (cache) {
        cache->rewindToEnd();
      } else {
        ret = lseek(fd, 0, SEEK_END);
      }
    }
    if (ret == -1) {
      std::ostringstream err;
      err << "Can' seek in file '" << name << "': " << strerror(errno);
      throw std::runtime_error(err.str());
    }
  }
  if (active && (direction != _direction)) {
    if (!bytes_read && prev_bytes_read) {
      bytes_read = prev_bytes_read;
    }
    int offset = bytes_read;
    if (direction == Direction::Forward) {
      offset *= -1;
    }
    if (offset) {
      if (cache) {
        cache->offsetTo(offset);
      } else if (lseek(fd, offset, SEEK_CUR) == -1) {
        std::ostringstream err;
        err << "Can' seek in file '" << name << "': " << strerror(errno);
        throw std::runtime_error(err.str());
      }
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
  return readBackward(symbol);
}

void FileIO::unread() {
  if (cache) {
    if (direction == Direction::Forward) {
      cache->offsetTo(-1);
    } else {
      cache->offsetTo(1);
    }
    return;
  }
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
