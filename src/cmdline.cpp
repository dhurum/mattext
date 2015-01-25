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
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "cmdline.h"

#define DEFAULT_DELAY 80
#define DEFAULT_BLOCK_LINES 1
#define _MAKE_STR(a) #a
#define MAKE_STR(a) _MAKE_STR(a)

static argp_option options[] = {
  {"delay", 'd', "value",  0, "Delay between redraws in milliseconds, default "
    MAKE_STR(DEFAULT_DELAY), 1},
  {"rand-len", 'l', "value",  0, "Max length of random symbols columns", 1},
  {"onepage", 'o', NULL,  0, "Show only one page", 2},
  {"non-interact", 'n', NULL,  0, "Run in non-interactive mode", 2},
  {"infinite", 'i', NULL,  0,
    "At the end of file start reading it from the beginning", 2},
  {"block-lines", 'b', "value",  0,
    "Block until at least specified number of lines is read, default "
      MAKE_STR(DEFAULT_BLOCK_LINES), 2},
  {"block-page", 'B', NULL,  0,
    "Block until full page is read", 2},
  {"colorize", 'c', NULL,  0, "Colorize output", 3},
  {"center-horiz", 'C', NULL,  0, "Center text horizontally", 3},
  {"center-horiz-longest", 'L', NULL,  0,
    "Center text horizontally by longest string", 3},
  {"center-vert", 'v', NULL,  0, "Center text vertically", 3},
  {"without-japanese", 'e', NULL,  0, "Do not use Japanese symbols", 4},
  {0}
};

static bool getValue(int &value, char *str)
{
  char *end = NULL;
  int res = strtol(str, &end, 0);

  if(str == end)
  {
    return false;
  }
  value = res;
  return true;
}

static error_t parseOptions(int key, char *arg, struct argp_state *state)
{
  CmdLineArgs *args = (CmdLineArgs*)state->input;

  switch (key)
  {
    case 'd':
      if(!getValue(args->delay, arg))
      {
        return ARGP_ERR_UNKNOWN;
      }
      break;
    case 'l':
      if(!getValue(args->rand_columns_len, arg))
      {
        return ARGP_ERR_UNKNOWN;
      }
      break;
    case 'o':
      args->onepage = true;
      break;
    case 'n':
      args->noninteract = true;
      break;
    case 'i':
      args->infinite = true;
      break;
    case 'c':
      args->colorize = true;
      break;
    case 'C':
      args->center_horiz = true;
      break;
    case 'L':
      args->center_horiz = true;
      args->center_horiz_longest = true;
      break;
    case 'v':
      args->center_vert = true;
      break;
    case 'e':
      args->without_japanese = true;
      break;
    case 'b':
      if(!getValue(args->block_lines, arg))
      {
        return ARGP_ERR_UNKNOWN;
      }
      break;
    case 'B':
      args->block_lines = -1;
      break;
    case 0:
      args->addFile(arg);
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

CmdLineArgs::CmdLineArgs(int argc, char *argv[])
{
  delay = DEFAULT_DELAY;
  rand_columns_len = 0;
  block_lines = DEFAULT_BLOCK_LINES;
  onepage = false;
  noninteract = false;
  colorize = false;
  center_horiz = false;
  center_horiz_longest = false;
  center_vert = false;
  without_japanese = false;
  infinite = false;

  stdin_returned = false;
  real_files = false;

  argp argp_opts = {options, parseOptions, "file1 file2 ..."};
  argp_parse (&argp_opts, argc, argv, 0, 0, this);
  
  if(real_files && files.empty())
  {
    fprintf(stderr, "Can't open any of given files!\n");
    exit(1);
  }
}

CmdLineArgs::~CmdLineArgs()
{
  for(list<FILE*>::iterator it = files.begin(); it != files.end(); ++it)
  {
    fclose(*it);
  }
}

void CmdLineArgs::addFile(char *name)
{
  size_t len = strlen(name);
  if(!len)
  {
    return;
  }

  real_files = true;
  FILE *file = fopen(name, "r");

  if(!file)
  {
    fprintf(stderr, "Can't open file %s!\n", name);
    return;
  }
  int flags = fcntl(fileno(file), F_GETFL, 0);
  fcntl(fileno(file), F_SETFL, flags | O_NONBLOCK);

  files.push_back(file);
  file_it = files.begin();
}

FILE* CmdLineArgs::getNextFile()
{
  if(files.empty())
  {
    if(stdin_returned && !infinite)
    {
      return NULL;
    }
    if(!stdin_returned)
    {
      int flags = fcntl(fileno(stdin), F_GETFL, 0);
      fcntl(fileno(stdin), F_SETFL, flags | O_NONBLOCK);
    }

    stdin_returned = true;
    return stdin;
  }

  if(file_it == files.end())
  {
    if(!infinite)
    {
      return NULL;
    }
    file_it = files.begin();
  }

  fseek(*file_it, 0,SEEK_SET);

  FILE *ret = *file_it;
  ++file_it;

  return ret;
}
