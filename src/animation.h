#pragma once

#include <ev++.h>
#include <vector>
#include <functional>
#include "config.h"
#include "terminal.h"
#include "text_stream.h"

class Animation {
 public:
  Animation(const Config &config, const Terminal &terminal);
  void play(const Text &text, std::function<void()> on_stop);
  void stop();
  bool isPlaying();
  void tick(ev::timer &w, int revents);

 private:
  const Config &config;
  const Terminal &terminal;
  const Text *text;
  ev::timer timer_watcher;
  bool is_playing = false;
  std::function<void()> on_stop;
  std::vector<size_t> col_lengths;
  std::vector<size_t> col_offsets;
  size_t max_col_length;
  size_t tick_id;
  size_t tail_length = 10;

  void init();
  wchar_t getRandSymbol();
};
