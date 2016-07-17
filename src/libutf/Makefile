# Filename: src/libutf/Makefile
# Project: libutf
# Brief: Build a subset of the Plan9 UTF library.
#
# Copyright (C) 2016 Guy Shaw
# Written by Guy Shaw <gshaw@acm.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c, %.o, $(SOURCES))

CC := gcc
CPPFLAGS := -I../inc
CFLAGS := -std=c99 -Wall -Wextra -g
LIB := libutf

.PHONY: all install clean show-targets

all: $(LIB).a

$(LIB).a: $(OBJECTS)
	ar crv $(LIB).a $(OBJECTS)

clean:
	rm -f $(LIB).a $(OBJECTS) *.o

show-targets:
	@show-makefile-targets

__show-%:
	@echo $*=$($*)
