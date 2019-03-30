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
    Date   : Thu Oct 19 06:02:01 IST 2017

******************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <limpid/common.h>
#include <limpid/config.h>

string_t *new_string3(int len, char *data, int data_len)
{
    if (len == data_len)
        len++;  // Hack to allow for null termintion.

    string_t *s = calloc(1, sizeof(string_t) + len);
    if (s == NULL) {
        fprintf(stderr, "limpid: error at alloc for string_t\n");
        return NULL;
    }

    s->max_len = len;
    s->len = 0;
    if (data && data_len) {
        memcpy(s->arr, data, data_len);
        s->len = data_len;
    }
    return s;
}

int string_printf(string_t *str, char *mode, const char *fmt, ...)
{
    int ret;
    va_list (args);
    va_start(args, fmt);
    int st = (mode[0] != 'a') ? 0 : str->len;
    ret = vsnprintf(str->arr + st, str->max_len - st, fmt, args);
    va_end(args);
    if ((str->len + ret) >= str->max_len) {
        str->len = str-> max_len;
    } else {
        str->len += ret;
        str->arr[str->len] = 0;
    }
    return ret;
}

int string_append(string_t *str, char *mode, char *buf, int len)
{
    if ( str->len >= str->max_len || str->len < 0)
        return -1;

    if (mode[0] == 'f')
        len = str->max_len - str->len;

    if ((str->len + len) > str->max_len)
        return -1;

    int i, ovf=0;
    for (i=0; i<len; i++) {
        if (ovf) break;
        str->arr[str->len] = buf[i];
        str->len++;
        if(str->len >= str->max_len)
            ovf = 1;
    }

    if (!ovf) {
        /* We'll try to null terminate if the string hasn't
         * already over flown. This is only a added feature
         * so we won't be policing it.
         */
        str->arr[str->len] = 0;
    }
    return i;
}
