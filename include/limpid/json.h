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
    Date   : Sat May 12 23:37:06 IST 2018

******************************************************************************/

#ifndef _LIMPID_JSON_H
#define _LIMPID_JSON_H

#include <limpid/common.h>
#include <limpid/lib-json.h>
#include <limpid/lib-string.h>

#define LIMPID_REG_JSON(x,y)	\
({				\
	lhandle_t h;		\
	h.type=LHANDLE_JSON;	\
	h.trigger=x;		\
	h.json_handle=y;	\
	limpid_register(&h);	\
})

int limpid_send_json_cmd(string_t *cmd, string_t **resp);

#endif
