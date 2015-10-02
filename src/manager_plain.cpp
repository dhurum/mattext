#include <wchar.h>
#include <stdexcept>
#include <sstream>
#include <ev++.h>
#include <string.h>
#include "manager_plain.h"

ManagerPlain::ManagerPlain(const Config &config, TextStream &text_stream,
                           const Terminal &terminal)
    : config(config), text_stream(text_stream), terminal(terminal) {
  read();
}

ManagerPlain::~ManagerPlain() {}

void ManagerPlain::read() {
  text_stream.read(
      [this](const Text &text) {
        while (true) {
          const wchar_t *line = text.getLine();
          if (!line) {
            this->read();
            return;
          }
          if (fputws(line, stdout) == -1) {
            std::ostringstream err;
            err << "Can't write to stdout: " << strerror(errno);
            throw std::runtime_error(err.str());
          }
        }
      },
      [this]() { this->quit(); }, terminal.getWidth(), terminal.getHeight());
}

void ManagerPlain::quit() {
  ev::get_default_loop().break_loop(ev::ALL);
}
