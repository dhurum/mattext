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

#include <argp.h>
#include <stdlib.h>
#include "config.h"

#define _MAKE_STR(a) #a
#define MAKE_STR(a) _MAKE_STR(a)

static argp_option options[] = {
    {"delay", 'd', "value", 0,
     "Delay between redraws in milliseconds, default " MAKE_STR(DEFAULT_DELAY),
     1},
    {"rand-len", 'l', "value", 0, "Max length of random symbols columns", 1},
    {"non-interact", 'n', NULL, 0, "Run in non-interactive mode", 2},
    {"infinite", 'i', NULL, 0,
     "At the end of file start reading it from the beginning", 2},
    {"block-lines", 'b', "value", 0,
     "Block until at least specified number of lines is read, "
     "default " MAKE_STR(DEFAULT_BLOCK_LINES),
     2},
    {"block-page", 'B', NULL, 0, "Block until full page is read", 2},
    {"colorize", 'c', NULL, 0, "Colorize output", 3},
    {"center-horiz", 'C', NULL, 0, "Center text horizontally", 3},
    {"center-horiz-longest", 'L', NULL, 0,
     "Center text horizontally by longest string", 3},
    {"center-vert", 'v', NULL, 0, "Center text vertically", 3},
    {"without-japanese", 'e', NULL, 0, "Do not use Japanese symbols", 4},
    {0}};

static bool getIntArg(int &value, char *str) {
  char *end = NULL;
  int res = strtol(str, &end, 0);

  if (str == end) {
    return false;
  }
  value = res;
  return true;
}

static error_t parseOptions(int key, char *arg, struct argp_state *state) {
  Config *config = (Config *)state->input;

  switch (key) {
    case 'd':
      if (!getIntArg(config->delay, arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      break;
    case 'l':
      if (!getIntArg(config->rand_columns_len, arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      break;
    case 'n':
      config->noninteract = true;
      break;
    case 'i':
      config->infinite = true;
      break;
    case 'c':
      config->use_colors = true;
      break;
    case 'C':
      config->center_horiz = true;
      break;
    case 'L':
      config->center_horiz = true;
      config->center_horiz_longest = true;
      break;
    case 'v':
      config->center_vert = true;
      break;
    case 'e':
      config->without_japanese = true;
      break;
    case 'b':
      if (!getIntArg(config->block_lines, arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      break;
    case 'B':
      config->block_lines = -1;
      break;
    case ARGP_KEY_ARG:
      config->files.push_back(arg);
      break;
    default:
      break;
  }
  return 0;
}

Config::Config(int argc, char *argv[]) {
  argp argp_opts = {options, parseOptions, "file[, file, ...]"};
  argp_parse(&argp_opts, argc, argv, 0, 0, this);
}

#if 0
void Config::addFile(char *name) {
  size_t len = strlen(name);
  if (!len) {
    return;
  }

  real_files = true;
  FILE *file = fopen(name, "r");

  if (!file) {
    fprintf(stderr, "Can't open file %s!\n", name);
    return;
  }
  int flags = fcntl(fileno(file), F_GETFL, 0);
  fcntl(fileno(file), F_SETFL, flags | O_NONBLOCK);

  files.push_back(file);
  file_it = files.begin();
}

FILE *Config::getNextFile() {
  if (files.empty()) {
    if (stdin_returned && !config::infinite) {
      return NULL;
    }
    if (!stdin_returned) {
      int flags = fcntl(fileno(stdin), F_GETFL, 0);
      fcntl(fileno(stdin), F_SETFL, flags | O_NONBLOCK);
    }

    stdin_returned = true;
    return stdin;
  }

  if (file_it == files.end()) {
    if (!config::infinite) {
      return NULL;
    }
    file_it = files.begin();
  }

  fseek(*file_it, 0, SEEK_SET);

  FILE *ret = *file_it;
  ++file_it;

  return ret;
}
#endif
