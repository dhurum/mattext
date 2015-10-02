#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include "manager_interactive.h"

ManagerInteractive::ManagerInteractive(const Config &config,
                                       TextStream &text_stream,
                                       const Terminal &terminal)
    : config(config),
      text_stream(text_stream),
      terminal(terminal),
      animation(config, terminal) {
  tty_fno = open("/dev/tty", O_RDONLY);
  fcntl(tty_fno, F_SETFL, fcntl(tty_fno, F_GETFL, 0) | O_NONBLOCK);

  io_watcher.set<ManagerInteractive, &ManagerInteractive::inputCb>(this);
  io_watcher.start(tty_fno, ev::READ);

  getNextPage();
}

ManagerInteractive::~ManagerInteractive() {
  close(tty_fno);
}

void ManagerInteractive::inputCb(ev::io &w, int revents) {
  int cmd = 0;

  int ret = read(tty_fno, &cmd, sizeof(int));

  if (ret == -1) {
    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
      return;
    }
    std::ostringstream err;
    err << "Can't read from terminal: " << strerror(errno);
    throw std::runtime_error(err.str());
  }

  if (!ret) {
    quit();
  }

  switch (cmd) {
    case 'q':
    case 'Q':
    case '\4':
      quit();
      break;
    default:
      getNextPage();
      break;
  }
}

void ManagerInteractive::checkPending() {
  if (next_page_pending || config.noninteract) {
    next_page_pending = false;
    getNextPage();
  }
}

void ManagerInteractive::getNextPage() {
  if (animation.isPlaying()) {
    next_page_pending = true;
    return;
  }
  text_stream.read([this](const Text &text) {
    this->animation.play(text, [this]() { this->checkPending(); });
  }, nullptr, terminal.getWidth(), terminal.getHeight());
}

void ManagerInteractive::quit() {
  io_watcher.stop();
  text_stream.stop();
  animation.stop();
  io_watcher.loop.break_loop(ev::ALL);
}
