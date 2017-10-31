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

#include <limpid.h>

#define tty_erase_line() write(1, "\033[K", 3)
#define tty_save_cursor() write(1, "\033[s", 3)
#define tty_restore_cursor() write(1, "\033[u", 3)
#define tty_send_newline() write(1, "\r\n", 2)
#define tty_send_string(x) write(1, x, strlen(x))
#define tty_flush() fflush(stdout)
#define tty_qeury_cursor() write(1, "\033[6n", 4)
#define tty_put_char(c) write(1, c, 1)
#define tty_move_cursor_forward(x) \
	do{fprintf(stdout, "\033[%dC", x);fflush(stdout);}while(0)
#define tty_move_cursor_backward(x) \
	do{fprintf(stdout, "\033[%dD", x); fflush(stdout);}while(0)
#define tty_set_cursor(r, c) \
	do{fprintf(stdout, "\033[%d;%dH", r, c); fflush(stdout);}while(0)

static int start_row, start_col, backup_sate;
struct termios temios_backup;

static int tty_get_cursor(int *row, int *col)
{
	int retval=-1;
	char *p, buf[8];

	do {
		tty_qeury_cursor();
		read (0 ,buf ,sizeof(buf));
		if (buf[0] != '\033' || buf[1] != '[')
			break;
		p = buf + 2;
		*row = atoi(p);
		while(p && *(p++) != ';');
		if (p == NULL)
			break;
		*col = atoi(p);
		while(p && *(++p) != 'R');
		if (p == NULL)
			break;
		retval = 0;
	} while (0);

	return retval;
}

static void tty_set_sane()
{
	if (backup_sate) {
		// Put back tty to sane.
		if(tcsetattr(0, TCSAFLUSH, &temios_backup) < 0) {
			fprintf(stderr, "Error: Cannot reset terminal!\n");
			return;
		}
	}
}

static int tty_set_raw()
{
	// Set raw mode on stdin.
	struct termios new;

	if (backup_sate == 0) {
		if(tcgetattr(0, &temios_backup) < 0) {
			fprintf(stderr, "Error: Unable to backup termios.\n");
			return -1;
		}
		backup_sate = 1;
	}

	new = temios_backup;
	new.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	new.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	new.c_cflag &= ~(CSIZE | PARENB);
	new.c_cflag |= CS8;
	new.c_oflag &= ~(OPOST);
	new.c_cc[VMIN] = 1;
	new.c_cc[VTIME] = 0;

	if(tcsetattr(0, TCSAFLUSH, &new) < 0)
		return -1;

	return 0;
}

static inline void redraw_line(const char *buf)
{
	tty_save_cursor();
	tty_set_cursor(start_row, start_col);
	tty_erase_line();
	tty_send_string(buf);
	tty_restore_cursor();
	tty_flush();
}

/*
** Return 1 to continue parse loop, 0 to proceed.
*/
static int parse_csi(int *state, int c, char *buf, int *cursor)
{
	if (*state == 0) {
		if (c == 0x1b) {
			*state = 1;
			return 1;
		}
		return 0;
	}
	if (*state == 1) {
		*state = (c == '[') ? 2 : 0;
		return 1;
	}
	if (*state == 2) {
		switch(c) {
		case 'A': // Arrow Up
			*state = 0;
			break;
		case 'B': // Arrow Down
			*state = 0;
			break;
		case 'D': // Left Arrow
			if (*cursor) {
				tty_move_cursor_backward(1);
				(*cursor)--;
			}
			*state = 0;
			break;
		case 'C': // Right Arrow
			if (buf[*cursor]) {
				tty_move_cursor_forward(1);
				(*cursor)++;
			}
			*state = 0;
			break;
		}
		return 1;
	}
	*state = 0;
	return 0;
}

static char *read_line_raw()
{
	int i, c, tmp;
	int csi_state = 0, tab_count = 0, cursor = 0;

	// Get starting offset to know boundaries
	if (tty_get_cursor(&start_row, &start_col)) {
		fprintf(stderr, "Error: unable to grab current position\n");
		return NULL;
	}

	char *buf = calloc(LIMPID_READ_LINE_MAXLEN, sizeof(char));
	if (buf == NULL) {
		fprintf(stderr, "Error: unable to allocate memory\n");
		return NULL;
	}

	while((i = read(0, &c, 1)) == 1) {

		c &= 0xff;

#if 1
		if (parse_csi(&csi_state, c, buf, &cursor))
			continue;
#endif
		switch (c)  {
		case 0x0d: // Return key
			tty_send_newline();
			return buf;
		case 0x03: // Ctrl-C
			tty_send_string("^C");
			tty_send_newline();
			free(buf);
			return NULL;
		case 0x01: // Ctrl-A
			while (cursor) {
				tty_move_cursor_backward(1);
				cursor--;
			}
			continue;
		case 0x05: // Ctrl-E
			while (buf[cursor]) {
				tty_move_cursor_forward(1);
				cursor++;
			}
			continue;
		case 0x08: // Backspace
			continue;
		case 0x15: // Ctrl-U
			tmp=0;
			do {
				buf[tmp++] = buf[cursor];
			} while (buf[cursor++]);
			cursor = 0;
			tty_set_cursor(start_row, start_col);
			redraw_line(buf);
			continue;
		case 0x17: // Ctrl-W
			if (!cursor)
				continue;
			tmp = cursor - 1;
			while (tmp >= 0 && buf[tmp] == ' ') tmp--;
			while (tmp >= 0 && buf[tmp] != ' ') tmp--;
			tmp++;
			buf[tmp] = 0;
			tty_move_cursor_backward(cursor-tmp);
			cursor = tmp;
			redraw_line(buf);
			continue;
		case 0x7f: // Delete (behaves as backspace)
			if (!cursor)
				continue;
			tmp = cursor;
			do {
				buf[tmp-1] = buf[tmp];
			} while (buf[tmp++]);
			cursor--;
			tty_move_cursor_backward(1);
			redraw_line(buf);
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

		if (c < 0x20 && c > 0x7E) {
			// Un-handled, non-printable characters.
			continue;
		}

		// Maxlen violation. Truncate and exit.
		if (strlen(buf) >= LIMPID_READ_LINE_MAXLEN) {
			buf[LIMPID_READ_LINE_MAXLEN-1] = 0;
			break;
		}

		// Cursor always points to end of buf. If cursor is not
		// pointing to '\0' we are inserting inline.

		if (buf[cursor] == 0) {
			// Insert at end.
			buf[cursor++] = c;
			buf[cursor] = 0;
			// Just print that char alone, as the rest of
			// the line hasn't changed.
			tty_put_char(&c);
			continue;
		}

		// Insert at middle
		tmp = strlen(buf) + 1;
		while(tmp >= cursor) {
			buf[tmp] = buf[tmp-1];
			tmp--;
		}
		// insert c but not tailing '\0' as we
		// expect it to be already present.
		buf[cursor++] = c;
		// We just added one character.
		// move the cursor forward once.
		tty_move_cursor_forward(1);

		redraw_line(buf);
	}

	if( i < 0) {
		fprintf(stderr, "Error: Raw mode read error.\n");
		return NULL;
	}

	return buf;
}

void read_line_reset()
{
	tty_set_sane();
}

char *read_line(const char *prompt)
{
	char *line;

	if (tty_set_raw()) {
		fprintf(stderr, "Error: Can't go to raw mode.\n");
		return NULL;
	}

	tty_send_string(prompt);

	line = read_line_raw();

	tty_set_sane();

	return line;
}
