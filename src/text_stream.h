#pragma once

#include <list>
#include <functional>
#include <vector>
#include <ev++.h>
#include <stdio.h>
#include "config.h"
#include "terminal.h"

class Text {
  friend class TextStream;

 public:
  Text(const Config &config, const Terminal &terminal);
  wchar_t get(size_t column, size_t row) const noexcept;
  const wchar_t *getLine() const noexcept;

 private:
  const Config &config;
  const Terminal &terminal;

  std::vector<std::vector<wchar_t>> lines;
  std::vector<size_t> lines_len;
  size_t lines_read;
  size_t longest_line_len;
  mutable size_t cur_out_line;

  void reset(size_t line_max_len, size_t lines_num);
};

class TextStream {
 public:
  TextStream(const Config &config, const Terminal &terminal);
  ~TextStream();
  void readCb(ev::io &w, int revents);
  void stop();
  void read(std::function<void(const Text &text)> on_read,
            std::function<void()> on_end, size_t line_max_len,
            size_t lines_num);

 private:
  const Config &config;
  const Terminal &terminal;
  std::list<std::pair<FILE *, const char *>> files;
  std::list<std::pair<FILE *, const char *>>::iterator current_file;
  ev::io io_watcher;
  Text text;
  size_t line_max_len;
  size_t lines_num;
  bool prev_line_finished;
  std::function<void(const Text &text)> on_read;
  std::function<void()> on_end;

  void addFile(FILE *file, const char *name);
  bool tryRewind();
};
