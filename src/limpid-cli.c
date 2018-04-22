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
    Date   : Sun Apr 22 10:13:37 IST 2018

******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <limpid.h>

struct limpid_cmd_s {
	char *trigger;
	int (*cmd_handler)(int argc, char **argv, string_t **resp);
};

int limpid_num_cmds;
struct limpid_cmd_s *limpid_cmd[LIMPID_MAX_COMMANDS];

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

	if (args && *args && !argv) free(args);

	*argc = argn;
	return argv;
}

static void free_parsed_args(char **argv)
{
	if (argv) {
		free(argv[-1]);	// args
		free(argv-1);	// argv
	}
}

int limpid_process_cli_cmd(lchunk_t *cmd, lchunk_t **resp)
{
	string_t *resp_str=NULL;
	char *p=NULL, **argv=NULL, cmd_buf[256];
	int i, argc, len=0;

	if (resp) *resp = NULL;
	memcpy(cmd_buf, cmd->data, cmd->length);
	cmd_buf[cmd->length] = 0;
	argv = parse_args(cmd_buf, &argc);

	for (i=0; i<limpid_num_cmds; i++) {
		if (strcmp(cmd->trigger, limpid_cmd[i]->trigger) != 0)
			continue;
		limpid_cmd[i]->cmd_handler(argc, argv, &resp_str);
		p = resp_str->arr;
		len = resp_str->len;
		*resp = limpid_make_chunk(TYPE_RESPONSE, cmd->trigger, p, len);
		free(resp_str);
		break;
	}

	free_parsed_args(argv);
	return i >= limpid_num_cmds ? -1 : 0;
}

int limpid_register_cli_handle(const char *trigger, int (*handler)(int, char **, string_t **))
{
	int id;
	struct limpid_cmd_s *cmd;

	for (id=0; id<limpid_num_cmds; id++) {
		if (strcmp(trigger, limpid_cmd[id]->trigger) == 0)
			break;
	}

	if (id >= limpid_num_cmds) {
		if(limpid_num_cmds+1 >= LIMPID_MAX_COMMANDS)
			return -1;
		
	}

	cmd = calloc(1, sizeof(struct limpid_cmd_s));
	if (cmd == NULL) {
		fprintf(stderr, "limpid: failed to alloc memory for command\n");
		return -1;
	}

	id = limpid_num_cmds++;
	cmd->trigger = strdup(trigger);
	cmd->cmd_handler = handler;
	limpid_cmd[id] = cmd;
	return 0;
}

int limpid_read_cli_cmd(const char *prompt, char **trigger, char **args)
{
	int i, j, ret;
	char *line, tmp[128];

	do {
		i = 0; j = 0; ret = 0;
		*trigger = NULL; *args = NULL;

		if ((line = read_line(prompt)) == NULL)
			break;

		if (line && (strlen(line) == 0))
			break;

		/* copy first word (trigger) */
		while (line[i] && line[i] == ' ') i++;	/* skip leading WS */
		while (line[i] && line[i] != ' ')
			tmp[j++] = line[i++];		/* copy non-WS chars */
		while (line[i] && line[i] == ' ') i++;	/* skip tailing WS */

		if (j == 0) break;

		tmp[j] = 0;

		if (strcmp(tmp, "exit") == 0) {
			ret = 1;
			break;
		}

		*trigger = strdup(tmp);

		if (strlen(line + i))
			*args = strdup(line + i);
		ret = strlen(tmp);

	} while(0);

	free(line);

	return ret;
}

int limpid_send_cli_cmd(char *trigger, char*args, char **resp)
{
	int ret = 1;

	limpid_t *ctx = limpid_connnect("/tmp/limpid-server");
	lchunk_t *c = limpid_make_chunk(TYPE_COMMAND, trigger,
			args, args ? strlen(args) : 0);

	if (limpid_send(ctx, c) < 0) {
		fprintf(stderr, "Failed to send command to server.\n");
		exit(EXIT_FAILURE);
	}

	if (limpid_receive(ctx, &c)) {
		fprintf(stderr, "Failed to receive response.\n");
		exit(EXIT_FAILURE);
	}

	do {
		if (c->length == 0) {
			fprintf(stderr, "limpid: unknown command '%s'\n", trigger);
			break;
		}

		if (resp == NULL)
			break;

		*resp = malloc(sizeof(char) * (c->length + 1));
		if (*resp == NULL) {
			fprintf(stderr, "Failed at malloc\n");
			exit(EXIT_FAILURE);
		}
		memcpy(*resp, c->data, c->length);
		*(resp + c->length) = 0;
		ret = 0;
	} while(0);

	free(c);
	limpid_disconnect(ctx);

	return ret;
}

