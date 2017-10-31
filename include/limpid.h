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

#ifndef _LIMPID_H
#define _LIMPID_H

// Configs
#define LIMPID_MAX_COMMANDS        128
#define LIMPID_READ_LINE_MAXLEN    128

typedef struct {
	int len;
	int maxLen;
	char arr[0];
} string_t;

#define newString(x,y) {0}; char x##arr[y]; x.arr=x##cArr; x.len=0; x.maxLen=y; x.reserved=0;
#define createString(x,y) char x##arr[y]; string_t x={.arr=x##arr, .len=0, .maxLen=y, .reserved=0 }

string_t *new_string(int len);
int string_printf(string_t *str, char *mode, const char *fmt, ...);
int string_append(string_t *str, char *mode, char *buf, int len);

char *read_line(const char *prompt);

enum {
	TYPE_COMMAND,
	TYPE_RESPONSE,
	TYPE_COMPLETION,
};

typedef struct {
	int type;       // see below
	int fd;         // client or server fd
	int client_fd;  // write-to-fd for server
	char *path;     // path to socket.
} limpid_t;

// limpid_t::type
#define LIMPID_SERVER  0
#define LIMPID_CLIENT  1

typedef struct {
	int type;
	string_t str;
} lchunk_t;

limpid_t *limpid_server_init(const char *path);
limpid_t *limpid_connnect(const char *path);
void limpid_disconnect(limpid_t *ctx);
lchunk_t *limpid_make_chunk(int type, char *buf, int len);
int limpid_send(limpid_t *ctx, lchunk_t *c);
int limpid_receive(limpid_t *ctx, lchunk_t **c);
int limpid_create_command(const char* cmd_str, int (*handler)(int, char **, string_t **));

#endif
