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

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <wchar.h>
#include "cmdline.h"
#include "input.h"
#include "screen.h"

#define STR_LEN 1024

static void simpleOutput(CmdLineArgs *args)
{
  wchar_t text[STR_LEN];
  FILE *file = args->getNextFile();

  while(true)
  {
    if(!fgetws(text, STR_LEN, file))
    {
      if(!(file = args->getNextFile()))
      {
        return;
      }
    }
    fputws(text, stdout);
  }
}

void animatedOutput(CmdLineArgs *args)
{
  Input input;
  Screen screen(args, &input);
  size_t min_lines_num = 0;
  FILE *file = args->getNextFile();

  if(args->block_lines < 0)
  {
    min_lines_num = screen.rows;
  }
  else
  {
    min_lines_num = args->block_lines;
  }

  while(true)
  {
    bool file_end = true;

    if(screen.readLines(file, min_lines_num, file_end) == Quit)
    {
      break;
    }

    if(file_end)
    {
      file = args->getNextFile();
      if(!file)
      {
        break;
      }
      if(file != stdin)
      {
        continue;
      }
    }
    InputAction cmd = screen.playAnimation();

    if(args->onepage)
    {
      break;
    }
    if(!args->noninteract && (cmd == WouldBlock))
    {
      cmd = input.get(true);
    }
    if(cmd == Quit)
    {
      break;
    }
  }
}

int main(int argc, char *argv[])
{
  CmdLineArgs args(argc, argv);
  srand(time(NULL));

  if(isatty(fileno(stdout)))
  {
    animatedOutput(&args);
  }
  else
  {
    simpleOutput(&args);
  }

  return 0;
}
