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

#include <string>
#include <argp.h>
#include <stdlib.h>
#include "config.h"
#include "animation.h"

#define _MAKE_STR(a) #a
#define MAKE_STR(a) _MAKE_STR(a)

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
    case 'a':
      config->animation_next = arg;
      config->animation_prev = arg;
      break;
    case -1:
      config->animation_next = arg;
      break;
    case -2:
      config->animation_prev = arg;
      break;
    default:
      break;
  }
  return 0;
}

Config::Config(int argc, char *argv[]) {
  static std::string animation_desc =
      "Animation for switching pages. Available animations are " +
      AnimationStore::getNames();
  static argp_option options[] = {
      {"delay", 'd', "value", 0,
       "Delay between redraws in milliseconds, default " MAKE_STR(
           DEFAULT_DELAY),
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
      {"animation", 'a', "name", 0, animation_desc.c_str(), 5},
      {"animation-next", -1, "name", 0, "Animation for showing next page", 5},
      {"animation-prev", -2, "name", 0, "Animation for showing previous page",
       5},
      {0}};

  argp argp_opts = {options, parseOptions, "file[, file, ...]"};
  argp_parse(&argp_opts, argc, argv, 0, 0, this);
}
