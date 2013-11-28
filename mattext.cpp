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
#include <list>

#define STR_LEN 1024
#define DEFAULT_DELAY 80
#define __MAKE_STR(a) #a
#define _MAKE_STR(a) __MAKE_STR(a)
#define DEFAULT_DELAY_STR _MAKE_STR(DEFAULT_DELAY)

using std::list;

struct CmdLineArgs
{
  int delay,
      rand_columns_len;
  bool onepage,
       noninteract,
       colorize,
       centrate_horiz,
       centrate_horiz_longest,
       centrate_vert,
       without_japanese,
       infinite;

  CmdLineArgs();
  ~CmdLineArgs();
  void addFile(char *name);
  FILE* getNextFile();
  bool checkFiles();

  private:
    list<FILE*> files;
    list<FILE*>::iterator file_it;
    bool stdin_returned,
         real_files;
};

CmdLineArgs::CmdLineArgs()
{
  delay = DEFAULT_DELAY;
  rand_columns_len = 0;
  onepage = false;
  noninteract = false;
  colorize = false;
  centrate_horiz = false;
  centrate_horiz_longest = false;
  centrate_vert = false;
  without_japanese = false;
  infinite = false;

  stdin_returned = false;
  real_files = false;
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

  files.push_back(file);
  file_it = files.begin();
}

bool CmdLineArgs::checkFiles()
{
  if(real_files && files.empty())
  {
    return false;
  }
  return true;
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

class Screen
{
  public:
    size_t rows,
           cols;
    Screen(CmdLineArgs *args);
    ~Screen();
    void playAnimation(int *usr_cmd);
    void setTextInfo(wchar_t *text, size_t *strings_lens, size_t *longest_str,
            size_t *read_strings);
    void waitForInput(int *usr_cmd);

  private:
    int *col_lengths,
        *col_offsets,
        max_col_length,
        tail_length,
        tty_fno,
        delay;
    wchar_t *first_chars,
            *text;
    size_t *strings_lens,
           *longest_str,
           *read_strings;
    fd_set fds;
    struct timeval select_tm;
    bool colorize,
         centrate_horiz,
         centrate_horiz_longest,
         centrate_vert,
         without_japanese;

    wchar_t getTextSymbol(int row, int col);
    bool checkInput();
    wchar_t getRandomSymbol();
    void updateSymbol(int col, int row, wchar_t symbol, bool bold);
};

Screen::Screen(CmdLineArgs *args)
{
  delay = args->delay;

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

wchar_t Screen::getRandomSymbol()
{
  int base = without_japanese ? 3 : 4;

  switch(rand() % base)
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

void Screen::updateSymbol(int col, int row, wchar_t symbol, bool bold = false)
{
  cchar_t out_char;
  int pair = colorize ? 1 : 0;

  setcchar(&out_char, &symbol, bold? A_BOLD : A_NORMAL, pair, NULL);
  mvadd_wch(col, row, &out_char);
}

wchar_t Screen::getTextSymbol(int row, int col)
{
  int start_row = centrate_vert ? ((rows - *read_strings) / 2) : 0;
  row -= start_row;

  size_t str_len = centrate_horiz_longest ? *longest_str : strings_lens[row];
  int start_col = centrate_horiz ? ((cols - str_len) / 2) : 0;
  col -= start_col;


  if((row < 0) || (row >= (int)*read_strings)
      || (col < 0) || (col >= (int)strings_lens[row]))
  {
    return ' ';
  }
  return text[row * cols + col];
}

static bool checkQuit(int *usr_cmd)
{
  if((*usr_cmd == 'q') || (*usr_cmd == 'Q') || (*usr_cmd == '\4'))
  {
    return true;
  }
  return false;
}

void Screen::playAnimation(int *usr_cmd)
{
  bool animation_started = false;
  bool animation_stopped = false;

  for(size_t i = 0; i < cols; ++i)
  {
    col_lengths[i] = 1 + rand() % max_col_length;
    col_offsets[i] = rand() % max_col_length;
  }

  for(size_t i = 0; true; ++i)
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
}

void Screen::setTextInfo(wchar_t *_text, size_t *_strings_lens,
    size_t *_longest_str, size_t *_read_strings)
{
  text = _text;
  strings_lens = _strings_lens;
  longest_str = _longest_str;
  read_strings = _read_strings;
}

void Screen::waitForInput(int *usr_cmd)
{
  read(tty_fno, usr_cmd, sizeof(int));
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

static struct argp_option options[] = {
  {"delay", 'd', "value",  0, "Delay between redraws in milliseconds, default "
    DEFAULT_DELAY_STR, 1},
  {"rand-len", 'l', "value",  0, "Max length of random symbols columns", 1},
  {"onepage", 'o', NULL,  0, "Show only one page", 2},
  {"non-interact", 'n', NULL,  0, "Run in non-interactive mode", 2},
  {"infinite", 'i', NULL,  0,
    "At the end of file start reading it from the beginning", 2},
  {"colorize", 'c', NULL,  0, "Colorize output", 3},
  {"centrate-horiz", 'C', NULL,  0, "Centrate text horizontally", 3},
  {"centrate-horiz-longest", 'L', NULL,  0,
    "Centrate text horizontally by longest string", 3},
  {"centrate-vert", 'v', NULL,  0, "Centrate text vertically", 3},
  {"without-japanese", 'e', NULL,  0, "Do not use Japanese symbols", 4},
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
    case 'i':
      args->infinite = true;
      break;
    case 'c':
      args->colorize = true;
      break;
    case 'C':
      args->centrate_horiz = true;
      break;
    case 'L':
      args->centrate_horiz = true;
      args->centrate_horiz_longest = true;
      break;
    case 'v':
      args->centrate_vert = true;
      break;
    case 'e':
      args->without_japanese = true;
      break;
    case 0:
      args->addFile(arg);
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

int parseCmdLine(CmdLineArgs *args, int argc, char *argv[])
{
  argp argp_opts = {options, parseOptions, "file1 file2 ..."};

  return argp_parse (&argp_opts, argc, argv, 0, 0, args);
}

int main(int argc, char *argv[])
{
  char is_tty = isatty(fileno(stdout));
  Screen *screen = NULL;
  wchar_t *text = NULL;
  size_t *strings_lens;
  size_t longest_str;
  size_t read_strings = 0;
  CmdLineArgs args;
  FILE *file = NULL;

  parseCmdLine(&args, argc, argv);

  if(!args.checkFiles())
  {
      fprintf(stderr, "Can't open any of given files!\n");
      exit(1);
  }

  file = args.getNextFile();

  if(is_tty)
  {
    setlocale(LC_CTYPE, "");
    srand(time(NULL));
    screen = new Screen(&args);
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
    int usr_cmd = 0;
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

  return 0;
}