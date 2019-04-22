/******************************************************************************

                  Copyright (c) 2017 Siddharth Chandrasekaran

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

    Author : Siddharth Chandrasekaran
    Email  : siddharth@embedjournal.com
    Date   : Sat May  5 11:36:56 IST 2018

******************************************************************************/

#ifndef _LIMPID_CLI_H
#define _LIMPID_CLI_H

#include <limpid/common.h>
#include <string.h>

#define LIMPID_REG_CLI(x,y)     \
({                              \
    lhandle_t h;                \
    h.type=LHANDLE_CLI;         \
    h.trigger=x;                \
    h.cli_handle=y;             \
    limpid_register(&h);        \
})

#define LIMPID_SUB_CMD(s, m) do {                            \
        if (argc >= 1 && strcmp(argv[0],s)==0) {             \
            return m(argc-1, argv+1, resp);                  \
        }                                                    \
    } while(0)

int limpid_read_cli_cmd(const char *prompt, char **trigger, char **args);
int limpid_send_cli_cmd(char *trigger, char*args, char **resp);

#endif
