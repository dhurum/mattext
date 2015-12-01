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

#include <ev++.h>
#include <exception>
#include <memory>
#include <locale.h>
#include <stdlib.h>
#include "config.h"
#include "file_stream.h"
#include "terminal.h"
#include "manager_interactive.h"
#include "manager_plain.h"

int main(int argc, char *argv[]) {
  srand(time(NULL));
  setlocale(LC_CTYPE, "");

  try {
    Config config(argc, argv);
    Terminal terminal(config);
    FileStream file_stream(config, terminal);
    std::unique_ptr<Manager> manager;
    if (terminal.stdoutIsTty()) {
      manager =
          std::make_unique<ManagerInteractive>(config, file_stream, terminal);
    } else {
      manager = std::make_unique<ManagerPlain>(file_stream);
    }

    ev_run(EV_DEFAULT, 0);
  } catch (std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return 1;
  }

  return 0;
}
