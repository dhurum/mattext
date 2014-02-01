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

#define _XOPEN_SOURCE_EXTENDED
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <unistd.h>
#include <sys/select.h>
#include "screen.h"

Screen::Screen(CmdLineArgs *args, Input *input) : input(input)
{
  delay = args->delay;
  
  setlocale(LC_CTYPE, "");

  initscr();
  noecho();
  cbreak();
  curs_set(0);
  getmaxyx(stdscr, rows, cols);

  colorize = has_colors() ? args->colorize : false;
  centrate_horiz = args->centrate_horiz;
  centrate_horiz_longest = args->centrate_horiz_longest;
  centrate_vert = args->centrate_vert;
  without_japanese = args->without_japanese;

  if(colorize)
  {
     start_color();
     init_pair(1, COLOR_GREEN, COLOR_BLACK);
  }

  col_lengths = new int[cols];
  col_offsets = new int[cols];
  first_chars = new wchar_t[cols];
  //We need rows * cols characters + one trailing '\0';
  text = new wchar_t[rows * cols + 1];
  lines_len = new size_t[rows];
  longest_line = 0;
  read_lines = 0;

  if(!col_lengths || ! col_offsets || !first_chars)
  {
    fprintf(stderr, "Can't allocate memory!\n");
    exit(1);
  }

  if(args->rand_columns_len <= 0)
  {
    max_col_length = rows;
  }
  else
  {
    max_col_length = args->rand_columns_len;
  }
  tail_length = 10;

  if(colorize)
  {
    attr_set(A_NORMAL, 1, NULL);
    refresh();
  }
}

Screen::~Screen()
{
  delete[] text;
  delete[] lines_len;
  delete[] col_lengths;
  delete[] col_offsets;
  delete[] first_chars;

  endwin();
}

InputAction Screen::readLines(FILE *file, size_t min_lines_num, bool &file_end)
{
  fd_set fds;
  timeval select_tm;
  InputAction cmd = WouldBlock;
  wchar_t *text_tmp = text;

  file_end = false;
  read_lines = 0;
  memset(lines_len, 0, sizeof(size_t) * rows);
  longest_line = 0;

  while(true)
  {
    errno = 0;
    for(; (read_lines < rows) && fgetws(text_tmp, cols + 1, file);
        ++read_lines)
    {
      lines_len[read_lines] = wcslen(text_tmp);
      if(text_tmp[lines_len[read_lines] - 1] == '\n')
      {
        --lines_len[read_lines];
        text_tmp[lines_len[read_lines]] = '\0';
      }
      text_tmp += cols;

      if(lines_len[read_lines] > longest_line)
      {
        longest_line = lines_len[read_lines];
      }
    }

    if((read_lines != rows) && (errno != EWOULDBLOCK))
    {
        file_end = true;
    }

    cmd = input->get();
    
    if((read_lines < min_lines_num) && !file_end && (cmd != Quit))
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

wchar_t Screen::getRandomSymbol()
{
  int type = rand() % (without_japanese ? 600 : 1000);

  if(type < 100)
  {
    return 0x30 + rand() % 9;
  }
  else if(type < 300)
  {
    return 0x41 + rand() % 26;
  }
  else if(type < 600)
  {
      return 0x61 + rand() % 26;
  }
  return 0xff66 + rand() % 58;
}

void Screen::updateSymbol(int col, int row, wchar_t symbol, bool bold = false)
{
  cchar_t out_char;
  int pair = colorize ? 1 : 0;

  setcchar(&out_char, &symbol, bold? A_BOLD : A_NORMAL, pair, NULL);
  mvadd_wch(col, row, &out_char);
}

wchar_t Screen::getTextSymbol(int row, int col)
{
  int start_row = centrate_vert ? ((rows - read_lines) / 2) : 0;
  row -= start_row;

  size_t str_len = centrate_horiz_longest ? longest_line : lines_len[row];
  int start_col = centrate_horiz ? ((cols - str_len) / 2) : 0;
  col -= start_col;


  if((row < 0) || (row >= (int)read_lines)
      || (col < 0) || (col >= (int)lines_len[row]))
  {
    return ' ';
  }
  return text[row * cols + col];
}

InputAction Screen::playAnimation()
{
  bool animation_started = false;
  bool animation_stopped = false;
  InputAction cmd = WouldBlock;

  for(size_t i = 0; i < cols; ++i)
  {
    col_lengths[i] = 1 + rand() % max_col_length;
    col_offsets[i] = rand() % max_col_length;
  }

  for(size_t i = 0; true; ++i)
  {
    InputAction tmp_cmd = input->get();

    if(tmp_cmd != WouldBlock)
    {
      cmd = tmp_cmd;
    }

    if(cmd == Quit)
    {
      break;
    }

    if(animation_started && animation_stopped)
    {
      break;
    }
    animation_stopped = true;

    for(size_t j = 0; j < cols; ++j)
    {
      int col_start = i - col_offsets[j];
      int col_end = i - col_offsets[j] - col_lengths[j];
      int text_start = col_end - tail_length - 1;

      if((col_start < 0) || (text_start >= (int)rows))
      {
        continue;
      }

      if((col_start >= 1) && (col_start <= (int)rows))
      {
        bool bold = ((rand() % 100) > 90) ? true : false;
        updateSymbol(col_start - 1, j, first_chars[j], bold);
        animation_stopped = false;
      }

      if(col_start < (int)rows)
      {
        first_chars[j] = getRandomSymbol();
        updateSymbol(col_start, j, first_chars[j], true);
        animation_started = true;
        animation_stopped = false;
      }

      int row_id = rand() % cols;
      if((row_id >= col_end) && (row_id < col_start))
      {
        bool bold = ((rand() % 100) > 60) ? true : false;
        updateSymbol(row_id, j, getRandomSymbol(), bold);
        animation_stopped = false;
      }

      int fade_col_id = (col_end - 1) - rand() % tail_length;
      if((fade_col_id >= 0) && (fade_col_id < (int)rows))
      {
        updateSymbol(fade_col_id, j, getTextSymbol(fade_col_id, j));
        animation_stopped = false;
      }

      if((text_start >= 0) && (text_start < (int)rows))
      {
        updateSymbol(text_start, j, getTextSymbol(text_start, j));
        animation_stopped = false;
      }
    }
    refresh();
    usleep(delay * 1000);
  }
  return cmd;
}
