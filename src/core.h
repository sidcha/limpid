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

#include <stdint.h>

#include <limpid/common.h>

enum lchunk_type_e {
    TYPE_COMMAND,
    TYPE_RESPONSE,
    TYPE_COMPLETION,
};

typedef struct {
    int type;       // see below
    int fd;         // client or server fd
    int client_fd;  // write-to-fd for server
} limpid_t;

#define LENC_TYPE(x,y)     (x | (y << 8))
#define LDEC_PROC(x)       (x & 0xff)
#define LDEC_CMD(x)        ((x >> 8) & 0x0ff)

// limpid_t::type
#define LIMPID_SERVER  0
#define LIMPID_CLIENT  1

typedef struct {
    uint8_t version;
    uint8_t type;
    uint8_t magic;
    int8_t status;
    uint8_t checksum;
    uint32_t length;
    uint8_t data[0];
} lchunk_t;

limpid_t *limpid_connect();
void limpid_disconnect(limpid_t *ctx);

lchunk_t *limpid_make_chunk(int type, void *data, int len);

int limpid_send(limpid_t *ctx, lchunk_t *chunk);
int limpid_receive(limpid_t *ctx, lchunk_t **chunk);

#endif
