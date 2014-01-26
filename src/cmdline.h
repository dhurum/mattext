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

#ifndef __CMDLINE_H__
#define __CMDLINE_H__

#include <list>
#include <stdio.h>

using std::list;

struct CmdLineArgs
{
  int delay,
      rand_columns_len,
      block_lines; /*0 - don't block, <0 - block page, >0 - block lines*/
  bool onepage,
       noninteract,
       colorize,
       centrate_horiz,
       centrate_horiz_longest,
       centrate_vert,
       without_japanese,
       infinite;

  CmdLineArgs(int argc, char *argv[]);
  ~CmdLineArgs();
  void addFile(char *name);
  FILE* getNextFile();

  private:
    list<FILE*> files;
    list<FILE*>::iterator file_it;
    bool stdin_returned,
         real_files;
};

#endif
