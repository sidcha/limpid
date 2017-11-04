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
    Date   : Sat Nov  4 21:31:56 IST 2017

******************************************************************************/

#include <unistd.h>

#include <limpid.h>

int cmd_ping(int argc, char *argv[], string_t **resp)
{
	int i;
	string_t *s = new_string(128);

	string_printf(s, "a", "pong");
	for (i=0; i<argc; i++) {
		string_printf(s, "a", "\n[%d] %s", i, argv[i]);
	}

	*resp = s;
	return 0;
}

int main(int argc, char *argv[])
{
	limpid_server_init("/tmp/limpid-server");

	limpid_create_command("ping", cmd_ping);

	while (1) {
		// Your application code!
	}
	return 0;
}

