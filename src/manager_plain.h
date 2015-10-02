#pragma once

#include "config.h"
#include "text_stream.h"
#include "terminal.h"
#include "manager.h"

class ManagerPlain : public Manager {
 public:
  ManagerPlain(const Config &config, TextStream &text_stream,
               const Terminal &terminal);
  ~ManagerPlain() override;
  void read();
  void quit();

 private:
  const Config &config;
  TextStream &text_stream;
  const Terminal &terminal;
};
