# 
#                 Copyright (c) 2017 Siddharth Chandrasekaran
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# 
#   Author : Siddharth Chandrasekaran
#   Email  : siddharth@embedjournal.com
#   Date   : Sat May 12 20:23:12 IST 2018
# 

ROOT_DIR ?= ../..

CFLAGS    = -Wall -I$(ROOT_DIR)/include -g3 -O
LDFLAGS   = -pthread -L$(ROOT_DIR)/lib -llimpid

all: app client

app: sample_app.o
	@echo "$(CC): linking $@..."
	@$(CC) -o $@ $^ $(LDFLAGS)

client: client.o
	@echo "$(CC): linking $@..."
	@$(CC) -o $@ $^ $(LDFLAGS)

clean:
	@rm -rf *.o app client

%.o: %.c
	@echo "$(CC): building $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all app client clean

