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
    Date   : Sat May 12 12:44:05 IST 2018

******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <limpid/core.h>
#include <limpid/common.h>

struct limpid_json_cmd_s {
	char *trigger;
	int (*cmd_handler)(json_t *cmd, string_t *resp);
};

int limpid_num_json_cmds;
struct limpid_json_cmd_s *limpid_json_cmd[LIMPID_MAX_COMMANDS];

int limpid_process_json_cmd(lchunk_t *cmd, lchunk_t **resp)
{
	string_t *resp_str;	
	int ret=-1, i;

	if (resp) *resp = NULL;

	resp_str = new_string(512);
	string_t *s = new_string3(cmd->length, (char *)cmd->data, cmd->length);
	json_t *json = json_parse(s);
	if (json == NULL) {
		printf("json-proc failed at parse! %s\n", s->arr);
		free(s);
		free(resp_str);
		return -1;
	}

	for (i=0; i<limpid_num_json_cmds; i++) {
		if (strcmp(cmd->trigger, limpid_json_cmd[i]->trigger) == 0)
			break;
	}
	if (i < limpid_num_json_cmds) {
		json_start(resp_str);
		json_printf(resp_str, "command", "%s", cmd->trigger);
		limpid_json_cmd[i]->cmd_handler(json, resp_str);
		json_end(resp_str);
		*resp = limpid_make_chunk(LENC_TYPE(LHANDLE_JSON, TYPE_RESPONSE),
				cmd->trigger, resp_str->arr, resp_str->len);
		ret = 0;
	}
	free(resp_str);
	return ret;
}

int limpid_register_json_handle(const char *trigger, int (*handler)(json_t *, string_t *))
{
	int id;
	struct limpid_json_cmd_s *cmd;

	for (id=0; id<limpid_num_json_cmds; id++) {
		if (strcmp(trigger, limpid_json_cmd[id]->trigger) == 0)
			break;
	}

	if (id >= limpid_num_json_cmds) {
		if(limpid_num_json_cmds+1 >= LIMPID_MAX_COMMANDS)
			return -1;
	}

	cmd = calloc(1, sizeof(struct limpid_json_cmd_s));
	if (cmd == NULL) {
		fprintf(stderr, "limpid: failed to alloc memory for command\n");
		return -1;
	}

	id = limpid_num_json_cmds++;
	cmd->trigger = strdup(trigger);
	cmd->cmd_handler = handler;
	limpid_json_cmd[id] = cmd;
	return 0;
}

int limpid_send_json_cmd(string_t *json, string_t **resp)
{
	char trigger[128];
	int ret = 1;
	string_t *value;

	limpid_t *ctx = limpid_connnect("/tmp/limpid-server");

	if (json_find(json, "command", &value)) {
		fprintf(stderr, "limpid: unable to find key command in json\n");
		limpid_disconnect(ctx);
		return -1;
	}

	if (value->len >= 128) {
		fprintf(stderr, "limpid: key command value exceeds 128 bytes\n");
		limpid_disconnect(ctx);
		return -1;
	}

	strncpy(trigger, value->arr, value->len);
	trigger[value->len] = 0;
	free(value);

	lchunk_t *c = limpid_make_chunk(LENC_TYPE(LHANDLE_JSON, TYPE_COMMAND),
			trigger, json, json ? json->len : 0);

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

		*resp = new_string3(c->length, (char *)c->data, c->length);
		if (*resp == NULL) {
			fprintf(stderr, "Failed at malloc\n");
			exit(EXIT_FAILURE);
		}
		ret = 0;
	} while(0);

	free(c);
	limpid_disconnect(ctx);
	return ret;
}

