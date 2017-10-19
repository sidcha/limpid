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
#include <unistd.h>
#include <termios.h>

#include "read_line.h"

#define tty_erase_line() write(1, "\033[K", 3)
#define tty_save_cursor() write(1, "\033[s", 3)
#define tty_restore_cursor() write(1, "\033[u", 3)
#define tty_move_cursor_forward(x) fprintf(stdout, "\033[%dC", x); fflush(stdout)
#define tty_move_cursor_backward(x) fprintf(stdout, "\033[%dD", x); fflush(stdout)
#define tty_set_cursor(r, c) fprintf(stdout, "\033[%d;%dH", r, c); fflush(stdout)
#define tty_send_newline() write(1, "\r\n", 2)
#define tty_send_string(x) write(1, x, strlen(x))
#define tty_flush() fflush(stdout)

int tty_get_cursor(int *row, int *col)
{
	int retval=0;
	char *p, buf[8];
	const char *cmd="\033[6n";

	do {
		write(1, cmd, 4);
		read (0 ,buf ,sizeof(buf));
		if (buf[0] != cmd[0] || buf[1] != cmd[1]) {
			retval=-2;
			break;
		}
		p = buf + 2;
		*row = atoi(p);
		while(p && *(p++) != ';');
		if (p == NULL) {
			retval=-3;
			break;
		}
		*col = atoi(p);
		while(p && *(++p) != 'R');
		if (p == NULL) {
			retval=-4;
			break;
		}
	} while (0);

	return retval;
}

char *read_line_raw()
{
	int i, c, tmp, start_row, start_col;
	int csi_state = 0, tab_count = 0, cursor = 0;

	// Get starting offset to know boundaries
	if (tty_get_cursor(&start_row, &start_col)) {
		fprintf(stderr, "Error: unable to grab current position\n");
		return NULL;
	}

	char *buf = calloc(LINE_LENGTH, sizeof(char));
	if (buf == NULL) {
		fprintf(stderr, "Error: unable to allocate %d bytes\n", LINE_LENGTH);
		return NULL;
	}

	while((i = read(0, &c, 1)) == 1) {

		c &= 0xff;

		switch(csi_state) {
		case 0:
			if (c == 0x1b) {
				csi_state++;
				continue;
			}
			break;
		case 1: 
			if (c == '[') {
				csi_state++;
				continue;
			}
			csi_state = 0;
			break;
		case 2: 
			switch(c) {
			case 'A': // Arrow Up
				csi_state = 0;
				break;
			case 'B': // Arrow Down
				csi_state = 0;
				break;
			case 'D': // Left Arrow
				if (cursor) {
					tty_move_cursor_backward(1);
					tty_flush();
					cursor--;
				}
				csi_state = 0;
				break;
			case 'C': // Right Arrow
				if (buf[cursor]) {
					tty_move_cursor_forward(1);
					cursor++;
				}
				csi_state = 0;
				break;
			}
			continue;
		}

		if (c == '\t') {
			if (tab_count < 2)
				tab_count++;
			// TODO: Implement tab completion.
			continue;
		} else {
			tab_count = 0;
		}

		switch (c)  {
		case 0x01: // Ctrl-A
			while (cursor) {
				tty_move_cursor_backward(1);
				cursor--;
			}
			continue;
		case 0x03: // Ctrl-C
			free(buf);
			return NULL;
		case 0x05: // Ctrl-E
			while (buf[cursor]) {
				tty_move_cursor_forward(1);
				cursor++;
			}
			continue;
		case 0x08: // Backspace
			break;
		case 0x15: // Ctrl-U
			tmp=0;
			do {
				buf[tmp++] = buf[cursor];
			} while (buf[cursor++]);
			cursor = 0;
			tty_set_cursor(start_row, start_col);
			break;
		case 0x17: // Ctrl-W
			if (cursor) {
				tmp = cursor - 1;
				while (tmp >= 0 && buf[tmp] == ' ') tmp--;
				while (tmp >= 0 && buf[tmp] != ' ') tmp--;
				tmp++;
				buf[tmp] = 0;
				tty_move_cursor_backward(cursor-tmp);
				cursor = tmp;
			}
			break;
		case 0x7f: // Delete (behaves as backspace)
			if (cursor) {
				tmp = cursor;
				do {
					buf[tmp-1] = buf[tmp];
				} while (buf[tmp++]);
				cursor--;
				tty_move_cursor_backward(1);
			}
			break;
		}

		if (c >= 0x20 && c <= 0x7E) {
			if (buf[cursor]) {
				// Cursor always points to end of buf.
				// if cursor is not pointing to '\0'
				// we are in insert mode.
				tmp = strlen(buf) + 1;
				while(tmp >= cursor) {
					buf[tmp] = buf[tmp-1];
					tmp--;
				}
				// insert c but not tailing '\0' as we
				// expect it to be already present.
				buf[cursor++] = c;
			} else {
				// Standard insert at end. Will need to
				// null terminate here.
				buf[cursor++] = c;
				buf[cursor] = 0;
			}

			tty_move_cursor_forward(1);

			if (strlen(buf) >= LINE_LENGTH) {
				buf[LINE_LENGTH-1] = 0;
				break;
			}
		}

		if (c == 0x0d) { // Return key
			tty_send_newline();
			break;
		}

		tty_save_cursor();
		tty_set_cursor(start_row, start_col);
		tty_erase_line();
		tty_send_string(buf);

		if (buf[cursor] != 0) {
			// When in insert mode, we will restore the cursor
			// in original place while chars get added to buf.
			tty_restore_cursor();
		}

		tty_flush();

	}

	if( i < 0) {
		fprintf(stderr, "Error: Raw mode read error.\n");
		return NULL;
	}

	return buf;
}

char *read_line(const char *prompt)
{
	printf("%s", prompt);
	fflush(stdout);

	// Set raw mode on stdin.
	struct termios new, old;
	if(tcgetattr(0, &old) < 0) {
		fprintf(stderr, "Error: Unable to backup termios.\n");
		return NULL;
	}

	new = old;
	new.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	new.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	new.c_cflag &= ~(CSIZE | PARENB);
	new.c_cflag |= CS8;
	new.c_oflag &= ~(OPOST);
	new.c_cc[VMIN] = 1;
	new.c_cc[VTIME] = 0;

	if(tcsetattr(0, TCSAFLUSH, &new) < 0) {
		fprintf(stderr, "Error: Can't go to raw mode.\n");
		return NULL;
	}

	char *line = read_line_raw();

	// Put back tty to sane.
	if(tcsetattr(0, TCSAFLUSH, &old) < 0) {
		fprintf(stderr, "Error: Cannot reset terminal!\n");
		return NULL;
	}

	return line;
}

