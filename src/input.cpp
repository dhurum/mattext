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

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "input.h"

Input::Input()
{
  select_tm.tv_sec = 0;
  select_tm.tv_usec = 0;
  tty_fno = open("/dev/tty", O_RDONLY);
}

Input::~Input()
{
  close(tty_fno);
}

InputAction Input::get(bool block)
{
  int usr_cmd;

  if(!block)
  {
    FD_ZERO(&fds);
    FD_SET(tty_fno, &fds);

    if(!select(tty_fno + 1, &fds, NULL, NULL, &select_tm))
    {
      return WouldBlock;
    }
  }

  read(tty_fno, &usr_cmd, sizeof(int));

  if((usr_cmd == 'q') || (usr_cmd == 'Q') || (usr_cmd == '\4'))
  {
    return Quit;
  }
  return NextPage;
}
