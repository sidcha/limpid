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
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include <limpid/core.h>

#ifdef LIMPID_USE_UNIX_SOCKETS
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#endif

#ifdef LIMPID_USE_TCP_SOCKETS
#include <sys/socket.h>
#include <netinet/in.h>
#endif


#define is_char(x) (((x) >= 'A' && (x) <= 'Z') || ((x) >= 'a' && (x) <='z'))
#define to_lower(x) do { if (is_char(x)) { x |= 0x20 } } while(0)

typedef int (*processor_t)(lchunk_t *, lchunk_t **);

// CLI Handler
int limpid_register_cli_handle(const char *trigger, int (*handler)(int, char **, string_t **));
int limpid_process_cli_cmd(lchunk_t *cmd, lchunk_t **resp);

// JSON Handler
int limpid_register_json_handle(const char *trigger, int (*handler)(json_t *, string_t *));
int limpid_process_json_cmd(lchunk_t *cmd, lchunk_t **resp);

processor_t processor[LHANDLE_SENTINEL] = {
	limpid_process_cli_cmd,
	limpid_process_json_cmd
};

void say(const char *fmt, ...)
{
	va_list arg;
	fprintf(stderr, "limpid: ");
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
}

#ifdef LIMPID_USE_UNIX_SOCKETS
int limpid_create_unix_socket(limpid_t *ctx)
{
	socklen_t len;
	struct sockaddr_un sock_serv;

	if ((ctx->fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		say("failed to open socket\n");
		return -1;
	}

	sock_serv.sun_family = AF_UNIX;
	strcpy(sock_serv.sun_path, LIMPID_SERVER_PATH);
	unlink(LIMPID_SERVER_PATH);
	len = sizeof(sock_serv.sun_family) + strlen(LIMPID_SERVER_PATH);

	if (bind(ctx->fd, (const struct sockaddr *)&sock_serv, len) < 0) {
		say("failed at bind!\n");
		return -1;
	}

	if (listen(ctx->fd, 5) < 0) {
		say("failed at listen!\n");
		return -1;
	}

	return 0;
}
#endif

#ifdef LIMPID_USE_TCP_SOCKETS
int limpid_create_tcp_socket(limpid_t *ctx)
{
	struct sockaddr_in sock_serv;

	if ((ctx->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		say("failed to open socket\n");
		return -1;
	}

	bzero((char *)&sock_serv, sizeof(sock_serv));
	sock_serv.sin_family = AF_INET;
	sock_serv.sin_addr.s_addr = INADDR_ANY;
	sock_serv.sin_port = htons(LIMPID_SERVER_PORT);

	if (bind(ctx->fd, (struct sockaddr *)&sock_serv, sizeof(sock_serv)) < 0) {
		say("failed to bind to port %d\n", LIMPID_SERVER_PORT);
		return -1;
	}

	if (listen(ctx->fd, 5) < 0) {
		say("failed at listen!\n");
		return -1;
	}

	return 0;
}
#endif

static void *limpid_listener(void *arg)
{
	int hnd, ret=-1;
	lchunk_t *cmd, *resp;
	socklen_t len;

	assert(arg);

	limpid_t *ctx = (limpid_t *) arg;

#ifdef LIMPID_USE_UNIX_SOCKETS
	struct sockaddr_un cli_addr;
	if (limpid_create_unix_socket(ctx))
		exit(EXIT_FAILURE);
#else
	struct sockaddr_in cli_addr;
	if (limpid_create_tcp_socket(ctx))
		exit(EXIT_FAILURE);
#endif
	while (1) {
		ctx->client_fd = accept(ctx->fd, (struct sockaddr *)&cli_addr, &len);
		if (ctx->client_fd < 0) {
			say("server - failed at accept\n");
			continue;
		}
		//printf("limpid: accepted connection from a clinet\n");
		cmd=NULL; resp=NULL;
		if (limpid_receive(ctx, &cmd)) {
			say("server - receive failed\n");
			close(ctx->client_fd);
			continue;
		}
		hnd = LDEC_PROC(cmd->type);
		if (hnd < LHANDLE_CLI && hnd >= LHANDLE_SENTINEL) {
			say("server - invalid command!\n");
			free(cmd);
			close(ctx->client_fd);
			continue;
		}
		ret = processor[hnd](cmd, &resp);
		free(cmd);
		if (resp == NULL) {
			// make a dummy resp if processor didn't return one.
			resp = limpid_make_chunk(TYPE_RESPONSE, NULL, 0);
		}
		resp->status = ret;
		if (limpid_send(ctx, resp)) {
			/* send failed, the client may be stuck endlessly 
			 * waiting for this data. We can do nothing but to close
			 * the connetion now and trigger a SIGPIPE and expect
			 * the other side to catch it.
			 */
			say("server - send failed\n");
			free(resp);
		}
		close(ctx->client_fd);
		//printf("limpid: closed connection to client\n");
	}

	// Will never reach this point.
	return NULL;
}

int limpid_server_init(const char *path)
{
	limpid_t *ctx = malloc(sizeof(limpid_t));
	assert(ctx);

	ctx->type = LIMPID_SERVER;
	pthread_t server_thread;
	pthread_create(&server_thread, NULL, limpid_listener, (void *)ctx);
	return 0;
}

limpid_t *limpid_connnect()
{
	limpid_t *ctx;
	
	if ((ctx = malloc(sizeof(limpid_t))) == NULL) {
		perror("limpid: client at alloc");
		exit(EXIT_FAILURE);
	}
	ctx->type = LIMPID_CLIENT;

#ifdef LIMPID_USE_UNIX_SOCKETS
	struct sockaddr_un serv_addr;
	if ((ctx->fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		free(ctx);
		return NULL;
	}

	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, LIMPID_SERVER_PATH);
	socklen_t sock_len = sizeof(serv_addr.sun_family) + 
					strlen(serv_addr.sun_path);
#else
	struct sockaddr_in serv_addr;
	if ((ctx->fd = socket(AF_INET , SOCK_STREAM , 0)) == -1) {
		perror("limpid: Failed at socket");
		exit(EXIT_FAILURE);
	}
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(LIMPID_SERVER_PORT);
	socklen_t sock_len = sizeof(serv_addr);
#endif

	if (connect(ctx->fd, (struct sockaddr *)&serv_addr, sock_len) != 0) {
		perror("limpid: failed at connect");
		exit(EXIT_FAILURE);
	}
	return ctx;
}

int limpid_send(limpid_t *ctx, lchunk_t *chunk)
{
	int i, fd, len;
	uint8_t *p = (uint8_t *)chunk, checksum;
	assert(ctx);
	assert(chunk);

	len = sizeof(lchunk_t) + chunk->length;
	chunk->checksum = 0; checksum = 0;
	for (i=0; i<len; i++)
		checksum ^= p[i];
	chunk->checksum = checksum;

	fd = (ctx->type == LIMPID_SERVER) ? ctx->client_fd : ctx->fd;
	if (write(fd, chunk, len) != len) {
		say("Error, unable to send data\n");
		return -1;
	}

	free(chunk);
	return 0;
}

int limpid_receive(limpid_t *ctx, lchunk_t **chunk)
{
	int i, fd, tout=0, read1, read2=-1, len;
	uint8_t *p, checksum, c_checksum=0;
	void *read_data;
	lchunk_t *c;

	assert(ctx);
	assert(chunk);

	fd = (ctx->type == LIMPID_SERVER) ? ctx->client_fd : ctx->fd;
	do {
		// Prevent partial reads by waiting for data
		// to stabilize.
		ioctl(fd, FIONREAD, &read1);
		if (read1 > 0) {
			usleep(100);
			ioctl(fd, FIONREAD, &read2);
		} else {
			if (++tout > 5000) {
				say("Read timeout!\n");
				return -1;
			}
			usleep(100);
		}
	} while (read1 != read2);

	if (read1 < sizeof(lchunk_t)) {
		say("Recveived deficit of lchunk\n");
		return -1;
	}

	read_data = malloc(read1);
	if (read_data == NULL) {
		say("Alloc error!\n");
		return -1;
	}

	if ((read(fd, read_data, read1)) < 0) {
		say("Error at read\n");
		free(read_data);
		return -1;
	}
	
	c = (lchunk_t *) read_data;
	checksum = c->checksum;
	c->checksum = 0;
	p = (uint8_t *)c;
	len = sizeof(lchunk_t) + c->length;
	for (i=0; i<len; i++) {
		c_checksum ^= p[i];
	}

	if (c_checksum != checksum) {
		say("checksum mismatch! exp:0x%02x calc:0x%02x\n",
				c_checksum, c_checksum);
		free(read_data);
		return -1;
	}

	if (c->version != LIMPID_API_VERSION) {
		say("API Version mismatch: got:%d exp:%d\n",
				c->version, LIMPID_API_VERSION);
		return -1;
	}

	*chunk = c;
	return 0;
}

void limpid_disconnect(limpid_t *ctx)
{
	close(ctx->fd);
	free(ctx);
}

lchunk_t *limpid_make_chunk(int type, void *data, int len)
{
	lchunk_t *c = calloc(1, sizeof(lchunk_t)+len);
	assert (c);

	c->checksum = 0; // computed/checked in send/receive
	c->version = LIMPID_API_VERSION;
	c->magic = LIMPID_MAGIC;
	c->type = type;
	c->length = len;
	memcpy(c->data, data, len);
	return c;
}

void limpid_register(lhandle_t *h)
{
	switch (h->type) {
	case LHANDLE_CLI:
		limpid_register_cli_handle(h->trigger, h->cli_handle);
		break;
	case LHANDLE_JSON:
		limpid_register_json_handle(h->trigger, h->json_handle);
		break;
	default:
		break;
	}
}
