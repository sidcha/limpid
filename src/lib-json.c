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
    Date   : Sat Nov  4 21:34:32 IST 2017

******************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <limpid/common.h>

#include "private.h"

enum {
    JSONP_JSON_START,       //1

    JSONP_KEY_WAIT,         //2
    JSONP_KEY_START,        //3
    JSONP_KEY,              //4
    JSONP_KEY_END,          //5

    JSONP_VAL_WAIT,         //6
    JSONP_VAL_START,        //7
    JSONP_VALUE,            //8
    JSONP_VAL_END,          //9

    JSONP_DONE,             //11
    JSONP_ERR               //12
};

static inline int json_parser_execute(int state, const char ch)
{
    if (state != JSONP_KEY && state != JSONP_VALUE) {
        if (ch == ' ' || ch == '\t' || ch == '\n')
            return state;
    }

    switch (state) {
    case JSONP_JSON_START:
        if (ch == '{') return JSONP_KEY_WAIT;
        break;
    case JSONP_KEY_WAIT:
        if (ch == '\'') return JSONP_KEY_START;
        break;
    case JSONP_VAL_WAIT:
        if (ch == '\'') return JSONP_VAL_START;
        break;
    case JSONP_KEY_START:
    case JSONP_KEY:
        if (ch == '\'') return JSONP_KEY_END;
        return JSONP_KEY;
    case JSONP_VAL_START:
    case JSONP_VALUE:
        if (ch == '\'') return JSONP_VAL_END;
        return JSONP_VALUE;
    case JSONP_KEY_END:
        if (ch == ':') return JSONP_VAL_WAIT;
        break;
    case JSONP_VAL_END:
        if (ch == '}') return JSONP_DONE;
        if (ch == ',') return JSONP_KEY_WAIT;
        break;
    default:
        printf("-%c-", ch);
        return state;
    }

    return JSONP_ERR;
}

static jsontok_t *json_tokenize(string_t *json, int *num_keys)
{
    jsontok_t *p, *t, *tmp;
    int i, cur_state, next_state, alloc_count = 10;
    char ch;

    t = p = calloc(1, sizeof(jsontok_t) * alloc_count);
    if (p == NULL) {
        return NULL;
    }

    *num_keys = 0;
    cur_state = next_state = JSONP_JSON_START;

    for (i = 0; i < json->len; i++) {
        ch = json->arr[i];
        next_state = json_parser_execute(cur_state, ch);
        //printf("dbg: cs,ns: [%d,%d] C: %c\n", cur_state, next_state, ch);
        switch (next_state) {
        case JSONP_KEY:
            if (t->klen == 0)
                t->kofs = i;
            t->klen++;
            break;
        case JSONP_VALUE:
            if (t->vlen == 0)
                t->vofs = i;
            t->vlen++;
            break;
        case JSONP_VAL_END:
            /* break if not a state tansition */
            if (cur_state == next_state)
                break;
            *num_keys += 1;
            if (*num_keys == alloc_count) {
                alloc_count += alloc_count;
                tmp = realloc(p, sizeof(jsontok_t) * alloc_count);
                if (tmp == NULL) {
                    free(p);
                    return NULL;
                }
                p = tmp;
            }
            t = p + *num_keys;
            break;
        case JSONP_DONE:
            return p;
        case JSONP_ERR:
            free(p);
            return NULL;
        }
        cur_state = next_state;
    }
    return NULL;
}

static inline int json_to_string(json_t *json, int key_num, string_t **key, string_t **val)
{
    jsontok_t *t = json->toks + key_num;
    string_t *p = json->json_str;

    *key = new_string3(t->klen, p->arr + t->kofs, t->klen);
    *val = new_string3(t->vlen, p->arr + t->vofs, t->vlen);
    return 0;
}

json_t *json_parse(string_t *json_str)
{
    json_t *json = malloc(sizeof(json_t));
    if (json == NULL) {
        printf("json Alloc error\n");
        return NULL;
    }

    json->toks = json_tokenize(json_str, &json->num_keys);
    if (json->toks == NULL) {
        printf("error at json_tokenize: %s\n", json_str->arr);
        free(json);
        return NULL;
    }

    json->json_str = json_str;
    return json;
}

void json_free(json_t *json)
{
    free(json->toks);
    free(json);
}

int json_find(string_t *json_str, const char *k, string_t **val)
{
    int ret=-1, i;
    string_t *key;

    json_t *j = json_parse(json_str);
    if (j == NULL)
        return -1;

    for (i=0; i<j->num_keys; i++) {
        json_to_string(j, i, &key, val);
        if (strncmp(k, key->arr, key->len) == 0) {
            free(key);
            ret = 0;
            break;
        }
        free(key);
        free(val);
    }

    json_free(j);
    return ret;
}

int json_strcmp(string_t *json_str, const char *k, const char *v)
{
    int ret = -1;
    string_t *val;

    if (json_find(json_str, k, &val) == 0 &&
            (strncmp(v, val->arr, val->len) == 0)) {
        ret = 0;
    }

    safe_free(val);
    return ret;
}

int json_sscanf(string_t *json_str, const char *k, const char *format, ...)
{
    int ret_val;
    va_list args;
    string_t *val;

    if (json_find(json_str, k, &val))
        return -1;

    va_start(args, format);
    ret_val = vsscanf(val->arr, format, args);
    va_end(args);

    safe_free(val);
    return ret_val;
}

int json_start(string_t *s)
{
    s->reserved = 0;
    string_append(s, "j", "{", 1); // j:join f:fill
    return s->reserved;
}

int json_end(string_t *s)
{
    string_append(s, "j", "}", 1); // j:join f:fill
    return s->reserved;
}

int json_printf(string_t *s, const char *key, const char *format, ...)
{
    char keyValStr[LIMPID_JSON_PRINT_BUFF_LEN];
    char keyFmtStr[LIMPID_JSON_PRINT_BUFF_LEN];
    va_list args;

    if (s->reserved) {
        if (string_append(s, "j", ",", 1) < 0)
            return -1;
    }

    int len = strlen(key) + strlen(format);

    // check if we can hold the entire key val pair.
    if (len > LIMPID_JSON_PRINT_BUFF_LEN - (2 +1 +2 +1) ) /* ['':'']\0 */
        return -1;

    // Now check if the target sting_t can hold it.
    // This need not be null terminated.
    if (len > (s->max_len - s->len - (2 +1 +2) ) ) /* ["":""] */
        return -1;

    snprintf(keyFmtStr, LIMPID_JSON_PRINT_BUFF_LEN, "'%s':'%s'", key, format);
    keyFmtStr[LIMPID_JSON_PRINT_BUFF_LEN-1] = 0; // just in case!!
    va_start(args, format);
    vsnprintf(keyValStr, LIMPID_JSON_PRINT_BUFF_LEN, keyFmtStr, args);
    keyValStr[LIMPID_JSON_PRINT_BUFF_LEN-1] = 0; // just in case!!
    va_end(args);

    string_append(s, "j", keyValStr, strlen(keyValStr));
    s->reserved++;

    return s->reserved;
}

void json_pprint(string_t *s)
{
    int i;
    string_t *key, *val;
    json_t *j;

    j = json_parse(s);
    if (j == NULL) {
        printf("error: malformed json: %s\n", s->arr);
        return;
    }

    printf("{\n");
    for (i=0; i<j->num_keys; i++) {
        if (i) printf(",\n");
        json_to_string(j, i, &key, &val);
        printf("\t'%s': '%s'", key->arr, val->arr);
        free(key);
        free(val);
    }
    printf("\n}\n");

    json_free(j);
}

#ifdef MODULE_TESTING
int main()
{
    string_t *s = new_string_const("{'Test1': 'Val1' , 'Test2': 'Val2'}");
    printf("json: %s\n", s->arr);
    json_pprint(s);
    free(s);
}
#endif
