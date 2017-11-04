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
#include <stdlib.h>
#include <string.h>

#include <limpid.h>

int main(int argc, char *argv[])
{
	char *line, resp[1024];


	while (1) {

		if ((line = read_line("[limpid] $ ")) == NULL) {
			continue;
		}

		if (strcmp(line, "exit") == 0) {
			exit(0);
		}

		if (strcmp(line, "devshell") == 0) {
			system("/bin/bash");
			continue;
		}

		if (line && (strlen(line) == 0)) {
			free(line);
			continue;
		}

		limpid_t *ctx = limpid_connnect("/tmp/limpid-server");

		if (ctx == NULL)  {
			fprintf(stderr, "Failed to connect to limpid server!\n");
			exit(-1);
		}

		lchunk_t *c = limpid_make_chunk(TYPE_COMMAND, line, strlen(line));

		if (limpid_send(ctx, c) < 0) {
			fprintf(stderr, "Failed to send command to server.\n");
			exit(-1);
		}

		if (limpid_receive(ctx, &c)) {
			fprintf(stderr, "Failed to receive response.\n");
			exit(-1);
		}

		if (c->str.len != 0) {
			strncpy(resp, c->str.arr, c->str.len);
			resp[c->str.len] = 0;
			printf("%s\n", resp);
		} else {
			fprintf(stderr, "limpid: '%s' unknown command\n", line);
		}

		free(c);
		free(line);
		line = NULL;

		limpid_disconnect(ctx);
	}


}
