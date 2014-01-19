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

#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <wchar.h>
#include "cmdline.h"
#include "input.h"
#include "screen.h"

#define STR_LEN 1024

int main(int argc, char *argv[])
{
  char is_tty = isatty(fileno(stdout));
  Screen *screen = NULL;
  Input *input = NULL;
  wchar_t *text = NULL;
  size_t *strings_lens;
  size_t longest_str;
  size_t read_strings = 0;
  CmdLineArgs args(argc, argv);
  FILE *file = NULL;

  file = args.getNextFile();

  if(is_tty)
  {
    setlocale(LC_CTYPE, "");
    srand(time(NULL));
    input = new Input();
    screen = new Screen(&args, input);
    //We need rows * cols characters + one trailing '\0';
    text = new wchar_t[screen->rows * screen->cols + 1];
    strings_lens = new size_t[screen->rows];
    screen->setTextInfo(text, strings_lens, & longest_str, &read_strings);
  }
  else
  {
    text = new wchar_t[STR_LEN];
  }

  while(true)
  {
    if(!is_tty)
    {
      if(!fgetws(text, STR_LEN, file))
      {
        break;
      }
      fputws(text, stdout);
      continue;
    }
    wchar_t *text_tmp = text;
    memset(strings_lens, 0, sizeof(size_t) * screen->rows);
    longest_str = 0;

    for(read_strings = 0;
        (read_strings < screen->rows) && fgetws(text_tmp, screen->cols + 1, file);
        ++read_strings)
    {
      strings_lens[read_strings] = wcslen(text_tmp);
      if(text_tmp[strings_lens[read_strings] - 1] == '\n')
      {
        --strings_lens[read_strings];
        text_tmp[strings_lens[read_strings]] = '\0';
      }
      text_tmp += screen->cols;
      if(strings_lens[read_strings] > longest_str)
      {
        longest_str = strings_lens[read_strings];
      }
    }
    if(!read_strings)
    {
      if(errno != EWOULDBLOCK)
      {
        file = args.getNextFile();
        if(!file)
        {
          break;
        }
        if(file != stdin)
        {
          continue;
        }
      }
    }
    InputAction cmd = screen->playAnimation();

    if(args.onepage)
    {
      break;
    }
    if(!args.noninteract && (cmd == WouldBlock))
    {
      cmd = input->get(true);
    }
    if(cmd == Quit)
    {
      break;
    }
  }

  delete[] text;
  if(is_tty)
  {
    delete[] strings_lens;
    delete input;
    delete screen;
  }

  return 0;
}
