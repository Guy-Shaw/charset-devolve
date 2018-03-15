
.PHONY: all diff-rune-table clean

all: rune-table.c

rune-table.c: Dumbdown.txt
	./build-table $^ > $@

diff-rune-table:
	diff -u ../../libdevolve/utf8/rune-table.c rune-table.c

clean:
	rm -f rune-table.c
