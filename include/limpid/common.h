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
    Date   : Sat May  5 11:36:41 IST 2018

******************************************************************************/

#ifndef _LIMPID_COMMON_H
#define _LIMPID_COMMON_H

#include <limpid/config.h>

/* --- from src/string.c --------------------------------------------------- */

typedef struct {
	int len;
	int max_len;
	char arr[0];
} string_t;

#define CREATE_STRING(x,y)	\
({				\
	char x##arr[y];		\
	string_t x = {		\
		.arr=x##arr,	\
		.len=0,		\
		.max_len=y	\
	}			\
})

string_t *new_string(int len);
int string_printf(string_t *str, char *mode, const char *fmt, ...);
int string_append(string_t *str, char *mode, char *buf, int len);

/* --- from src/string.c --------------------------------------------------- */

char *read_line(const char *prompt);

/* --- from src/limpid-core.c ---------------------------------------------- */
typedef enum {
	LHANDLE_CLI,
	LHANDLE_JSON,
	LHANDLE_SENTINEL
} lhandle_type_t;

typedef struct {
	lhandle_type_t type;
	const char *trigger;
	union {
		int (*cli_handle)(int, char **, string_t **);
	};
} lhandle_t;

void limpid_register(lhandle_t *h);
int limpid_server_init(const char *path);

#endif
