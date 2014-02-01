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
#include <sys/select.h>
#include "cmdline.h"
#include "input.h"
#include "screen.h"

#define STR_LEN 1024

static InputAction getLines(FILE *file, wchar_t *text, size_t line_max_len,
    size_t *lines_len, size_t &longest_line, size_t min_lines_num,
    size_t max_lines_num, size_t &lines_num, bool &file_end, Input *input)
{
  fd_set fds;
  timeval select_tm;
  InputAction cmd = WouldBlock;

  file_end = false;
  lines_num = 0;

  while(true)
  {
    errno = 0;
    for(; (lines_num < max_lines_num) && fgetws(text, line_max_len + 1, file);
        ++lines_num)
    {
      lines_len[lines_num] = wcslen(text);
      if(text[lines_len[lines_num] - 1] == '\n')
      {
        --lines_len[lines_num];
        text[lines_len[lines_num]] = '\0';
      }
      text += line_max_len;

      if(lines_len[lines_num] > longest_line)
      {
        longest_line = lines_len[lines_num];
      }
    }

    if((lines_num != max_lines_num) && (errno != EWOULDBLOCK))
    {
        file_end = true;
    }

    cmd = input->get();
    
    if((lines_num < min_lines_num) && !file_end && (cmd != Quit))
    {
      select_tm.tv_sec = 0;
      select_tm.tv_usec = 100;
      FD_ZERO(&fds);
      FD_SET(fileno(file), &fds);

      select(fileno(file) + 1, &fds, NULL, NULL, &select_tm);
    }
    else
    {
      break;
    }
  }
  return cmd;
}

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
  wchar_t *text = NULL;
  size_t *lines_len = NULL;
  size_t longest_line = 0;
  size_t read_lines = 0;
  size_t min_lines_num = 0;
  FILE *file = args->getNextFile();

  //We need rows * cols characters + one trailing '\0';
  text = new wchar_t[screen.rows * screen.cols + 1];
  lines_len = new size_t[screen.rows];
  screen.setTextInfo(text, lines_len, & longest_line, &read_lines);
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
    memset(lines_len, 0, sizeof(size_t) * screen.rows);
    longest_line = 0;
    bool file_end = true;

    if(getLines(file, text, screen.cols, lines_len, longest_line,
          min_lines_num, screen.rows, read_lines, file_end, &input) == Quit)
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

  delete[] text;
  delete[] lines_len;
}

int main(int argc, char *argv[])
{
  CmdLineArgs args(argc, argv);
  
  setlocale(LC_CTYPE, "");
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
