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
#include <readline/readline.h>
#include <readline/history.h>

#include <limpid/cli.h>

int limpid_read_cli_cmd(const char *prompt, char **trigger, char **args)
{
    int i, j, ret;
    char *line, tmp[128];

    do {
        i = 0; j = 0; ret = 0;
        *trigger = NULL; *args = NULL;

        if ((line = readline(prompt)) == NULL)
            return 0;

        if (line && (strlen(line) == 0))
            break;

        /* copy first word (trigger) */
        while (line[i] && line[i] == ' ') i++;    /* skip leading WS */
        while (line[i] && line[i] != ' ')
            tmp[j++] = line[i++];        /* copy non-WS chars */
        while (line[i] && line[i] == ' ') i++;    /* skip tailing WS */

        if (j == 0) break;

        tmp[j] = 0;

        if (strcmp(tmp, "exit") == 0) {
            ret = -1;
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

int main(int argc, char *argv[])
{
    int ret;
    char *trigger, *args, *resp;

    while (1) {
        ret = limpid_read_cli_cmd("[limpid]$ ", &trigger, &args);
        if (ret == 0) continue;
        if (ret == -1) break;

        if (limpid_send_cli_cmd(trigger, args, &resp) == 0) {
            printf("%s\n", resp);
            free(resp);
        }

        free(trigger);
        if (args) free(args);
    }

    return ret;
}

