
.PHONY: all clean

all: rune-table.c

rune-table.c: Dumbdown.txt
	./build-table $^ > $@

clean:
	rm -f rune-table.c
