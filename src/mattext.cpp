/*******************************************************************************

Copyright 2013, Denis Tikhomirov. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.

*******************************************************************************/

#include <ev++.h>
#include <exception>
#include <memory>
#include "config.h"
#include "text_stream.h"
#include "terminal.h"
#include "manager_interactive.h"
#include "manager_plain.h"

int main(int argc, char *argv[]) {
  srand(time(NULL));

  try {
    Config config(argc, argv);
    Terminal terminal(config);
    TextStream text_stream(config, terminal);
    std::unique_ptr<Manager> manager;
    if (terminal.isTty()) {
      manager =
          std::make_unique<ManagerInteractive>(config, text_stream, terminal);
    } else {
      manager = std::make_unique<ManagerPlain>(config, text_stream, terminal);
    }

    ev_run(EV_DEFAULT, 0);
  } catch (std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return 1;
  }

  return 0;
}
