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
#   Date   : Thu Oct 19 06:02:01 IST 2017
# 

MAJOR_NUMBER = 0
MINOR_NUMBER = 1

VERSION  := v$(MAJOR_NUMBER).$(MINOR_NUMBER)
ROOT_DIR := $(shell pwd)

CC       := $(CROSS_COMPILE)gcc
AR       := $(CROSS_COMPILE)ar

CFLAGS   = -Wall -Iinclude -DVERSION="$(VERSION)"
LDFLAGS  = -pthread -Llib -llimpid
PREFIX  ?= /usr

all: liblimpid example

liblimpid: lib/liblimpid.a

example:
	@echo "Building examples..."
	@make -C examples/cli all

clean:
	@make -C examples/cli clean
	@rm -rf lib/* src/*.o

install:
	@mkdir -p $(PREFIX)/lib/ $(PREFIX)/include/
	@cp -f lib/*.a $(PREFIX)/lib/
	@cp -rf include/* $(PREFIX)/include/

lib/liblimpid.a: src/limpid.o src/read_line.o src/string.o
	@mkdir -p lib
	@echo "$(AR): creating limpid library"
	@$(AR) rcs -o $@ $^

src/%.o: src/%.c
	@echo "$(CC): building $<"
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean

