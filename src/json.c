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

#include <limpid.h>

enum {
	JSONP_JSON_START,	//1

	JSONP_KEY_WAIT,		//2
	JSONP_KEY_START,	//3
	JSONP_KEY,		//4
	JSONP_KEY_END,		//5

	JSONP_VAL_WAIT,		//6
	JSONP_VAL_START,	//7
	JSONP_VALUE,		//8
	JSONP_VAL_END,		//9

	JSONP_DONE,		//11
	JSONP_ERR		//12
};

typedef struct {
	int kofs;
	int klen;
	int vofs;
	int vlen;
} jsontok_t;

typedef struct {
	int num_keys;
	string_t *json_str;
	jsontok_t *toks;
} json_t;

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
		if (ch == '"') return JSONP_KEY_START;
		break;
	case JSONP_VAL_WAIT:
		if (ch == '"') return JSONP_VAL_START;
		break;
	case JSONP_KEY_START:
	case JSONP_KEY:
		if (ch == '"') return JSONP_KEY_END;
		return JSONP_KEY;
	case JSONP_VAL_START:
	case JSONP_VALUE:
		if (ch == '"') return JSONP_VAL_END;
		return JSONP_VALUE;
	case JSONP_KEY_END:
		if (ch == ':') return JSONP_VAL_WAIT;
		break;
	case JSONP_VAL_END:
		if (ch == '}') return JSONP_DONE;
		if (ch == ',') return JSONP_KEY_WAIT;
		break;
	default:
		return state;
	}

	return JSONP_ERR;
}

jsontok_t *json_tokenize(string_t *json, int *num_keys)
{
	jsontok_t *p, *t, *tmp;
	int state=JSONP_JSON_START, alloc_count = 10;
	int i;
	
	
	t = p = malloc(sizeof(jsontok_t) * alloc_count);
	if (p == NULL) {
		return NULL;
	}

	*num_keys = 0;

	for (i = 0; i < json->len; i++) {
		switch (json_parser_execute(state, json->arr[i])) {
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
	}
	return NULL;
}

json_t *json_parse(string_t *json_str)
{
	json_t *json = malloc(sizeof(json_t));
	if (json == NULL)
		return NULL;


	json->toks = json_tokenize(json_str, &json->num_keys);
	if (json->toks == NULL) {
		free(json);
		return NULL;
	}

	json->json_str = json_str;
	return json;
}

int json_to_string(json_t *json, int key_num, string_t *key, string_t *val)
{
	if (key_num >= json->num_keys)
		return -1;

	jsontok_t *t = json->toks + key_num;
	string_t *p = json->json_str;

	key->arr = p->arr + t->kofs;
	key->len = key->max_len = t->klen;
	val->arr = p->arr + t->vofs;
	val->len = val->max_len = t->vlen;

	return 0;
}

int json_find(string_t *json_str, const char *k, string_t *val)
{
	int ret=-1, i;
	create_string(key, 64);

	json_t *j = json_parse(json_str);

	for (i=0; i<j->num_keys; i++) {
		if (json_to_string(j, i, &key, val))
			break;
		if (strncmp(k, key.arr, key.len) == 0) {
			ret = 0;
			break;
		}
	}

	free(j);
	return ret;
}

int json_strcmp(string_t *json_str, const char *k, const char *v)
{
	int ret = -1;
	create_string(val, 64);

	if (json_find(json_str, k, &val) == 0 &&
			(strncmp(v, val.arr, val.len) == 0)) {
		ret = 0;
	}

	return ret;
}

int json_sscanf(string_t *json_str, const char *k, const char *format, ...)
{
	create_string(val, 64);

	if (json_find(json_str, k, &val))
		return -1;

	va_list args;
	va_start(args, format);
	int retVal = vsscanf(val.arr, format, args);
	va_end(args);
	return retVal;
}

static int key_num;
int json_start(string_t *s)
{
	key_num = 0;
	string_append(s, "j", "{", 1); // j:join f:fill
	return key_num;
}

int json_end(string_t *s)
{
	string_append(s, "j", "}", 1); // j:join f:fill
	return key_num;
}

#define JSON_PRINT_BUFF 128
int json_printf(string_t *s, const char *key, const char *format, ...)
{
	char keyValStr[JSON_PRINT_BUFF], keyFmtStr[JSON_PRINT_BUFF];
	va_list(args);

	if (key_num) {
		if (string_append(s, "j", ",", 1) < 0)
			return -1;
	}

	int len = strlen(key) + strlen(format);
	
	// check if we can hold the entire key val pair.
	if (len > JSON_PRINT_BUFF - (2 -1 -2 -1 /* ["":""\0] */) )
		return -1;
	
	// Now check if the target sting_t can hold it.
	// This need not be null terminated.
	if (len > (s->max_len - s->len - (2 - 1 - 2 /* ["":""] */) ))
		return -1;

	snprintf(keyFmtStr, JSON_PRINT_BUFF, "\"%s\":\"%s\"", key, format);
	keyFmtStr[JSON_PRINT_BUFF-1] = 0; // just in case!!
	va_start(args, format);
	vsnprintf(keyValStr, JSON_PRINT_BUFF, keyFmtStr, args);
	keyValStr[JSON_PRINT_BUFF-1] = 0; // just in case!!
	va_end(args);

	string_append(s, "j", keyValStr, strlen(keyValStr));
	key_num++;
	
	return key_num;
}

