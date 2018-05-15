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

CFLAGS   = -Wall -Iinclude -DVERSION="$(VERSION)" -g3 -O
LDFLAGS  = -pthread -Llib -llimpid
PREFIX  ?= /usr

export

OBJ     := obj/limpid-core.o obj/lib-read-line.o obj/lib-string.o obj/lib-json.o
OBJ     += obj/limpid-cli.o obj/limpid-json.o

all: liblimpid example

liblimpid: lib/liblimpid.a

example:
	@echo "Building examples..."
	@make -s -C examples/cli all
	@make -s -C examples/json all

clean:
	@make -s -C examples/cli clean
	@make -s -C examples/json clean
	@rm -rf lib/* obj/ *.tar

install:
	@mkdir -p $(PREFIX)/lib/ $(PREFIX)/include/
	@cp -f lib/*.a $(PREFIX)/lib/
	@cp -rf include/* $(PREFIX)/include/

lib/liblimpid.a: $(OBJ)
	@mkdir -p lib
	@echo "$(AR): creating limpid library"
	@$(AR) rcs -o $@ $^

obj/%.o: src/%.c
	@test -d $@ || mkdir -p $(dir $@)
	@echo "$(CC): building $<"
	@$(CC) $(CFLAGS) -c $< -o $@

archive:
	@git archive --format=tar --prefix=limpid/ \
		--output=limpid-$$(git describe).tar master
	@tar -f limpid-$$(git describe).tar --delete limpid/.gitignore

.PHONY: all clean

