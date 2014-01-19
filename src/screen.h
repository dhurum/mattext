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

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "cmdline.h"
#include "input.h"

class Screen
{
  public:
    size_t rows,
           cols;
    Screen(CmdLineArgs *args, Input *input);
    ~Screen();
    InputAction playAnimation();
    void setTextInfo(wchar_t *text, size_t *strings_lens, size_t *longest_str,
            size_t *read_strings);

  private:
    int *col_lengths,
        *col_offsets,
        max_col_length,
        tail_length,
        delay;
    wchar_t *first_chars,
            *text;
    size_t *strings_lens,
           *longest_str,
           *read_strings;
    bool colorize,
         centrate_horiz,
         centrate_horiz_longest,
         centrate_vert,
         without_japanese;
    Input *input;

    wchar_t getTextSymbol(int row, int col);
    wchar_t getRandomSymbol();
    void updateSymbol(int col, int row, wchar_t symbol, bool bold);
};

#endif
