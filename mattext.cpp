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
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <argp.h>

#define STR_LEN 1024

class Screen
{
  public:
    int rows,
        cols;
    Screen(int delay, int rand_columns_len);
    ~Screen();
    void playAnimation(char *usr_cmd);
    void setTextInfo(wchar_t *text, size_t *strings_lens);
    void waitForInput(char *usr_cmd);

  private:
    int *col_lengths,
        *col_offsets,
        max_col_length,
        tail_length,
        tty_fno,
        delay;
    wchar_t *first_chars,
            *text;
    size_t *strings_lens;
    fd_set fds;
    struct timeval select_tm;

    wchar_t getTextSymbol(int row, int col);
    bool checkInput();
};

Screen::Screen(int _delay, int rand_columns_len)
{
  delay = _delay;

  initscr();
  noecho();
  cbreak();
  curs_set(0);
  getmaxyx(stdscr, rows, cols);

  col_lengths = new int[cols];
  col_offsets = new int[cols];
  first_chars = new wchar_t[cols];

  if(!col_lengths || ! col_offsets || !first_chars)
  {
    fprintf(stderr, "Can't allocate memory!\n");
    exit(1);
  }

  if(rand_columns_len <= 0)
  {
    max_col_length = rows;
  }
  else
  {
    max_col_length = rand_columns_len;
  }
  tail_length = 10;

  tty_fno = open("/dev/tty", O_RDONLY);
}

Screen::~Screen()
{
  delete[] col_lengths;
  delete[] col_offsets;
  delete[] first_chars;

  endwin();
  close(tty_fno);
}

static wchar_t getRandomSymbol()
{
  switch(rand() % 4)
  {
    case 0:
      return 0x30 + rand() % 9;
    case 1:
      return 0x41 + rand() % 26;
    case 2:
      return 0x61 + rand() % 26;
    default: 
      return 0xff66 + rand() % 58;
  }
}

static void updateSymbol(int col, int row, wchar_t symbol, bool bold = false)
{
  cchar_t out_char;

  setcchar(&out_char, &symbol, bold? A_BOLD : A_NORMAL, 0, NULL);
  mvadd_wch(col, row, &out_char);
}
    
wchar_t Screen::getTextSymbol(int row, int col)
{
  if((row >= rows) || (col >= strings_lens[row]))
  {
    return ' ';
  }
  return text[row * cols + col];
}

static bool checkQuit(char *usr_cmd)
{
  if((*usr_cmd == 'q') || (*usr_cmd == 'Q'))
  {
    return true;
  }
  return false;
}

void Screen::playAnimation(char *usr_cmd)
{
  bool animation_started = false;
  bool animation_stopped = false;

  for(int i = 0; i < cols; ++i)
  {
    col_lengths[i] = 1 + rand() % max_col_length;
    col_offsets[i] = rand() % max_col_length;
  }

  for(int i = 0; true; ++i)
  {
    if(checkInput())
    {
      waitForInput(usr_cmd);
      if(checkQuit(usr_cmd))
      {
        return;
      }
    }

    if(animation_started && animation_stopped)
    {
      break;
    }
    animation_stopped = true;
    
    for(int j = 0; j < cols; ++j)
    {
      int col_start = i - col_offsets[j];
      int col_end = i - col_offsets[j] - col_lengths[j];
      int text_start = col_end - tail_length - 1;

      if((col_start < 0) || (text_start >= rows))
      {
        continue;
      }

      if((col_start >= 1) && (col_start <= rows))
      {
        updateSymbol(col_start - 1, j, first_chars[j]);
        animation_stopped = false;
      }

      if(col_start < rows)
      {
        first_chars[j] = getRandomSymbol();
        updateSymbol(col_start, j, first_chars[j], true);
        animation_started = true;
        animation_stopped = false;
      }

      int row_id = rand() % cols;
      if((row_id >= col_end) && (row_id < col_start))
      {
        updateSymbol(row_id, j, getRandomSymbol());
        animation_stopped = false;
      }

      int fade_col_id = (col_end - 1) - rand() % tail_length;
      if((fade_col_id >= 0) && (fade_col_id < rows))
      {
        updateSymbol(fade_col_id, j, getTextSymbol(fade_col_id, j));
        animation_stopped = false;
      }

      if((text_start >= 0) && (text_start < rows))
      {
        updateSymbol(text_start, j, getTextSymbol(text_start, j));
        animation_stopped = false;
      }
    }
    refresh();
    usleep(delay * 1000);
  }
}

void Screen::setTextInfo(wchar_t *_text, size_t *_strings_lens)
{
  text = _text;
  strings_lens = _strings_lens;
}
    
void Screen::waitForInput(char *usr_cmd)
{
  read(tty_fno, usr_cmd, 1);
}
    
bool Screen::checkInput()
{
  FD_ZERO(&fds);
  FD_SET(tty_fno, &fds);
  select_tm.tv_sec = 0;
  select_tm.tv_usec = 0;

  if(select(tty_fno + 1, &fds, NULL, NULL, &select_tm))
  {
    return true;
  }
  return false;
}

struct CmdLineArgs
{
  int delay,
      rand_columns_len;
  bool onepage,
       noninteract;
  char filename[FILENAME_MAX + 1];
  CmdLineArgs();
};

CmdLineArgs::CmdLineArgs()
{
  delay = 80;
  rand_columns_len = 0;
  onepage = false;
  noninteract = false;
  filename[0] = '\0';
}

static struct argp_option options[] = {
  {"delay", 'd', "value",  0, "Delay between redraws in ms, default 80"},
  {"rand_len", 'l', "value",  0, "Max length of random symbols columns"},
  {"onepage", 'o', NULL,  0, "Show only one page"},
  {"non-interact", 'n', NULL,  0, "Run in non-interactive mode"},
  {0}
};

static error_t parseOptions(int key, char *arg, struct argp_state *state)
{
  CmdLineArgs *args = (CmdLineArgs*)state->input;

  switch (key)
  {
    case 'd':
      args->delay = atoi(arg);
      break;
    case 'l':
      args->rand_columns_len = atoi(arg);
      break;
    case 'o':
      args->onepage = true;
      break;
    case 'n':
      args->noninteract = true;
      break;
    case 0:
      strncpy(args->filename, arg, FILENAME_MAX);
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

int parseCmdLine(CmdLineArgs *args, int argc, char *argv[])
{
  argp argp_opts = {options, parseOptions};

  return argp_parse (&argp_opts, argc, argv, 0, 0, args);
}

int main(int argc, char *argv[])
{
  char is_tty = isatty(fileno(stdout));
  Screen *screen = NULL;
  wchar_t *text = NULL;
  size_t *strings_lens;
  CmdLineArgs args;
  FILE *file = stdin;

  parseCmdLine(&args, argc, argv);
  
  if(args.filename[0] != '\0')
  {
    file = fopen(args.filename, "r");
    if(!file)
    {
      fprintf(stderr, "Can't open file %s!\n", args.filename);
      exit(1);
    }
  }

  if(is_tty)
  {
    setlocale(LC_CTYPE, "");
    srand(time(NULL));
    screen = new Screen(args.delay, args.rand_columns_len);
    text = new wchar_t[screen->rows * screen->cols];
    strings_lens = new size_t[screen->rows];
    screen->setTextInfo(text, strings_lens);
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
    int str_num = 0;
    wchar_t *text_tmp = text;
    memset(strings_lens, 0, sizeof(size_t) * screen->rows);

    for(;(str_num < screen->rows) && fgetws(text_tmp, STR_LEN, file); 
        ++str_num)
    {
      strings_lens[str_num] = wcslen(text_tmp);
      if(text_tmp[strings_lens[str_num] - 1] == '\n')
      {
        --strings_lens[str_num];
        text_tmp[strings_lens[str_num]] = '\0';
      }
      text_tmp += screen->cols;
    }
    if(!str_num)
    {
      break;
    }
    char usr_cmd = 0;
    screen->playAnimation(&usr_cmd);

    if(args.onepage)
    {
      break;
    }
    if(!args.noninteract && (usr_cmd == 0))
    {
      screen->waitForInput(&usr_cmd);
    }
    if(checkQuit(&usr_cmd))
    {
      break;
    }
  }

  delete[] text;
  if(is_tty)
  {
    delete[] strings_lens;
    delete screen;
  }
  if(file != stdin)
  {
    fclose(file);
  }
  return 0;
}
