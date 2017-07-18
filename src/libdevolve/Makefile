# Filename: src/libdevolve/Makefile
# Project: libdevolve
# Brief: Modules for handling different character sets
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

LIBRARY := libdevolve
SUBLIBS := utf8 latin1

.PHONY: all clean show-targets

all: $(LIBRARY).a

$(LIBRARY).a:
	for dir in $(SUBLIBS) ; do ( cd $$dir && make ) ; done
	ar crv $(LIBRARY).a  utf8/*.o latin1/*.o

clean:
	rm -f $(LIBRARY).a
	for dir in $(SUBLIBS) ; do ( cd $$dir && make clean) ; done

show-targets:
	@show-makefile-targets

show-%:
	@echo $*=$($*)
