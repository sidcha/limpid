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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "read_line.h"

#define BUFSIZE 256
#define PS1 "shell> "

volatile int command_ready;
char command_buf[BUFSIZE];

enum {
	CLICMD_HELP,
	CLICMD_PING,
	CLICMD_EXIT,
	CLICMD_SENTINEL
};

struct cli_cmd {
	char *cmd_str;
	int (*cmd_handler)(int argc, char *argv[]);
};

int cmd_ping(int argc, char *argv[]);
int cmd_help(int argc, char *argv[]);
int cmd_exit(int argc, char *argv[]);

struct cli_cmd g_cli_cmd[CLICMD_SENTINEL] = {
	[CLICMD_HELP] = { "help", cmd_help },
	[CLICMD_PING] = { "ping", cmd_ping },
	[CLICMD_EXIT] = { "exit", cmd_exit },
};

int cmd_ping(int argc, char *argv[])
{
	int i;
	printf("pong\n");
	for (i=0; i<argc; i++) {
		printf("[%d] %s\n", i, argv[i]);
	}
	return 0;
}

int cmd_help(int argc, char *argv[])
{
	int i;
	printf("Commands: \n");
	for (i=0; i<CLICMD_SENTINEL; i++) {
		printf("%s\n", g_cli_cmd[i].cmd_str);
	}
	return 0;
}

int cmd_exit(int argc, char *argv[])
{
	exit(0);
	return 0;
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

char **parse_args(char *args, int *argc)
{
	char **argv = NULL;
	int    argn = 0;

	if (args && *args
			&& (args = strdup(args))
			&& (argn = set_args(args,NULL))
			&& (argv = malloc((argn+1) * sizeof(char *)))) {
		*argv++ = args;
		argn = set_args(args,argv);
	}

	if (args && !argv) free(args);

	*argc = argn;
	return argv;
}

void free_parsed_args(char **argv)
{
	if (argv) {
		free(argv[-1]);
		free(argv-1);
	} 
}

void process_command()
{
	char **av;
	int cmd_num, ac;

	if (command_ready == 0)
		return;

	av = parse_args(command_buf, &ac);
	if (ac < 1) {
		command_ready = 0;
		return;
	}

	for (cmd_num=0; cmd_num<CLICMD_SENTINEL; cmd_num++) {
		if (strcmp(av[0], g_cli_cmd[cmd_num].cmd_str) == 0)
			break;
	}

	if (cmd_num >= CLICMD_SENTINEL) {
		fprintf(stderr, "shell: %s unknown command\n", command_buf);
		command_ready = 0;
		return;
	}

	// invoke handler.
	g_cli_cmd[cmd_num].cmd_handler(ac-1, &av[1]);
	free_parsed_args(av);
	command_ready = 0;
}

void *cli_server(void *arg)
{
	int len;
	char *line;

	while (1) {
		line = read_line(PS1);

		if (line == NULL) continue;

		len = strlen(line);

		if(len == 0) continue;

		len = len < BUFSIZE ? len : BUFSIZE-1; 
		strncpy(command_buf, line, len);
		command_buf[len] = 0;
		free(line);
		command_ready = 1;
		
		// wait for the command to be read.
		while (command_ready == 1);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int tick = 0;
	pthread_t server_thread;
	pthread_create(&server_thread, NULL, cli_server, NULL);

	while (1) {
		tick++;
		if (tick > 10) {
			tick = 0;
		}
		process_command();
		usleep(100);
	}

	return 0;
}

