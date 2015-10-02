#pragma once

#include <ev++.h>
#include "config.h"
#include "text_stream.h"
#include "terminal.h"
#include "animation.h"
#include "manager.h"

class ManagerInteractive : public Manager {
 public:
  ManagerInteractive(const Config &config, TextStream &text_stream,
                     const Terminal &terminal);
  ~ManagerInteractive() override;
  void inputCb(ev::io &w, int revents);
  void checkPending();

 private:
  ev::io io_watcher;
  int tty_fno;
  const Config &config;
  TextStream &text_stream;
  const Terminal &terminal;
  Animation animation;
  bool next_page_pending = false;

  void getNextPage();
  void quit();
};
