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
    Date   : Sun May 13 09:22:07 IST 2018

******************************************************************************/

#ifndef _LIB_JSON_H
#define _LIB_JSON_H

#include <limpid/lib-string.h>

typedef struct {
    int kofs;
    int klen;
    int vofs;
    int vlen;
} jsontok_t;

typedef struct {
    int num_keys;
    string_t *json_str;
    jsontok_t *toks;
} json_t;

json_t *json_parse(string_t *json_str);
void json_free(json_t *json);

int json_find(string_t *json_str, const char *k, string_t **val);
int json_strcmp(string_t *json_str, const char *k, const char *v);
int json_sscanf(string_t *json_str, const char *k, const char *format, ...);

int json_start(string_t *s);
int json_end(string_t *s);
int json_printf(string_t *s, const char *key, const char *format, ...);

void json_pprint(string_t *);

#endif
