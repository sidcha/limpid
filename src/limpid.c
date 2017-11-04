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
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <limpid.h>

#define is_char(x) (((x) >= 'A' && (x) <= 'Z') || ((x) >= 'a' && (x) <='z'))
#define to_lower(x) do { if (is_char(x)) { x |= 0x20 } } while(0)

struct limpid_cmd_s {
	char *cmd_str;
	int (*cmd_handler)(int argc, char *argv[], string_t **s);
};

struct limpid_cmd_s *limpid_cmd[LIMPID_MAX_COMMANDS];
int limpid_num_cmds;

void say(const char *fmt, ...)
{
	va_list arg;
	fprintf(stderr, "Limpid: ");
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
}

static int set_args(char *args, char **argv)
{
	int count = 0;

	while (isspace(*args)) ++args;
	while (*args) {
		if (argv) argv[count] = args;
		while (*args && !isspace(*args)) ++args;
		if (argv && *args) *args++ = '\0';
		while (isspace(*args)) ++args;
		count++;
	}
	return count;
}

static char **parse_args(char *args, int *argc)
{
	char **argv = NULL;
	int argn = 0;

	if (args && *args && (args = strdup(args))
			&& (argn = set_args(args,NULL))
			&& (argv = malloc((argn+1) * sizeof(char *)))) {
		*argv++ = args;
		argn = set_args(args,argv);
	}

	if (args && !argv) free(args);

	*argc = argn;
	return argv;
}

static void free_parsed_args(char **argv)
{
	if (argv) {
		free(argv[-1]);
		free(argv-1);
	}
}

static int limpid_process_cmd(limpid_t *ctx, string_t *cmd)
{
	string_t *resp=NULL;
	lchunk_t *c;
	char *p=NULL, **av, cmd_buf[256];
	int i, ac, len=0;

	strncpy(cmd_buf, cmd->arr, cmd->len);
	cmd_buf[cmd->len] = 0;
	av = parse_args(cmd->arr, &ac);
	if (ac < 1) {
		fprintf(stderr, "limpid: error parsing command line\n");
		return -1;
	}

	for (i=0; i<limpid_num_cmds; i++) {
		if (strcmp(av[0], limpid_cmd[i]->cmd_str) == 0) {
			limpid_cmd[i]->cmd_handler(ac-1, &av[1], &resp);
			break;
		}
	}

	if (resp) {
		p = resp->arr;
		len = resp->len;
	}

	c = limpid_make_chunk(TYPE_RESPONSE, p, len);
	limpid_send(ctx, c);

	if (resp) free(resp);
	free_parsed_args(av);
	return 0;
}

int limpid_create_command(const char* cmd_str, int (*handler)(int, char **, string_t **))
{
	int id;
	struct limpid_cmd_s *cmd;

	if(limpid_num_cmds >= LIMPID_MAX_COMMANDS)
		return -1;

	cmd = malloc(sizeof(struct limpid_cmd_s));
	if (cmd == NULL) {
		say("failed to alloc memory for command\n");
		return -1;
	}

	id = limpid_num_cmds++;
	cmd->cmd_str = strdup(cmd_str);
	cmd->cmd_handler = handler;
	limpid_cmd[id] = cmd;
	return 0;
}

static void *limpid_listener(void *arg)
{
	struct sockaddr_un sock_serv, cli_addr;

	limpid_t *ctx = (limpid_t *) arg;

	if ((ctx->fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		say("failed to created server fd\n");
		return NULL;
	}

	sock_serv.sun_family = AF_UNIX;
	strcpy(sock_serv.sun_path, ctx->path);
	unlink(ctx->path);

	socklen_t len = sizeof(sock_serv.sun_family) + strlen(ctx->path);

	if (bind(ctx->fd, (const struct sockaddr *)&sock_serv, len) < 0) {
		say("failed at bind!\n");
		return NULL;
	}

	if (listen(ctx->fd, 5) < 0) {
		say("failed at listen!\n");
		return NULL;
	}

	while (1) {
		ctx->client_fd = accept(ctx->fd, (struct sockaddr *)&cli_addr, &len);
		if (ctx->client_fd < 0) {
			say("failed at accept\n");
			continue;
		}

		lchunk_t *c;
		if (limpid_receive(ctx, &c) == 0) {
			limpid_process_cmd(ctx, &c->str);
			free(c);
		}

		close(ctx->client_fd);
	}
	return NULL;
}

limpid_t *limpid_server_init(const char *path)
{
	limpid_t *ctx = malloc(sizeof(limpid_t));
	if (ctx == NULL)
		return NULL;

	ctx->type = LIMPID_SERVER;
	ctx->path = strdup(path);
	pthread_t server_thread;
	pthread_create(&server_thread, NULL, limpid_listener, (void *)ctx);
	return ctx;
}

limpid_t *limpid_connnect(const char *path)
{
	socklen_t sock_len;
	struct sockaddr_un serv_addr;

	limpid_t *ctx = malloc(sizeof(limpid_t));
	if (ctx == NULL)
		return NULL;

	ctx->type = LIMPID_CLIENT;
	ctx->path = NULL;

	if ((ctx->fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		free(ctx);
		return NULL;
	}

	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, path);

	sock_len = sizeof(serv_addr.sun_family) + strlen(serv_addr.sun_path);
	if (connect(ctx->fd, (const struct sockaddr *)&serv_addr, sock_len) != 0) {
		close(ctx->fd);
		free(ctx);
		return NULL;
	}
	ctx->path = strdup(path);
	return ctx;
}

void limpid_disconnect(limpid_t *ctx)
{
	close(ctx->fd);
	if (ctx->path)
		free(ctx->path);
	free(ctx);
}

lchunk_t *limpid_make_chunk(int type, char *buf, int len)
{
	lchunk_t *c = malloc(sizeof(lchunk_t)+len);

	if (c == NULL)
		return NULL;

	c->type = type;
	c->str.len = len;
	c->str.maxLen = len;
	memcpy(c->str.arr, buf, len);

	return c;
}

int limpid_send(limpid_t *ctx, lchunk_t *c)
{
	int ret = 0, fd, len;

	if (ctx == NULL || c == NULL)
		return -1;

	len = sizeof(lchunk_t) + c->str.len;

	fd = (ctx->type == LIMPID_SERVER) ? ctx->client_fd : ctx->fd;

	if ((ret = write(fd, c, len)) < 0) {
		say("Error at write");
	}

	free(c);
	return ret;
}

int limpid_receive(limpid_t *ctx, lchunk_t **c)
{
	int fd;
	volatile int read1, read2=-1;

	fd = (ctx->type == LIMPID_SERVER) ? ctx->client_fd : ctx->fd;

	do {
		// Prevent partial reads by waiting for data
		// to stabilize.
		ioctl(fd, FIONREAD, &read1);
		if (read1 > 0) {
			usleep(100);
			ioctl(fd, FIONREAD, &read2);
		}
		usleep(10);
	} while (read1 != read2);

	void *read_data = malloc(read1);

	if (read_data == NULL) {
		say("Alloc error!\n");
		return -1;
	}

	if ((read(fd, read_data, read1)) < 0) {
		say("Error at read\n");
		return -1;
	}

	*c = (lchunk_t *) read_data;
	return 0;
}

