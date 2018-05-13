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
    Date   : Sun May 13 09:23:22 IST 2018

******************************************************************************/

#ifndef _LIB_STRING_H_
#define _LIB_STRING_H_

#define CREATE_STRING(x,y)	\
({				\
	char x##arr[y];		\
	string_t x = {		\
		.arr=x##arr,	\
		.len=0,		\
		.max_len=y	\
	}			\
})
#define new_string(l)		new_string3(l, NULL, 0)
#define new_string_const(s)	new_string3(strlen(s), s, strlen(s))

typedef struct {
	int len;
	int max_len;
	int reserved;
	char arr[0];
} string_t;

string_t *new_string3(int len, char *data, int data_len);
int string_printf(string_t *str, char *mode, const char *fmt, ...);
int string_append(string_t *str, char *mode, char *buf, int len);

#endif
