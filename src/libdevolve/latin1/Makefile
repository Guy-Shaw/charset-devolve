# Filename: src/libdevolve/latin1/Makefile
# Project: libdevolve
# Brief: Modules for handling Latin1 character set
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
CONFIG := -DDEBUG
CPPFLAGS := -I../../inc
CFLAGS := -std=c99 -g -Wall -Wextra $(CONFIG) $(CPPFLAGS)

.PHONY: all clean show-targets

all: $(OBJECTS)

clean:
	rm -f *.o
	cscope-clean

show-targets:
	@show-makefile-targets

show-%:
	@echo $*=$($*)
