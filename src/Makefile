.PHONY: all test clean .FORCE

all: cmd/charset-devolve

cmd/charset-devolve: .FORCE
	cd libdevolve && make
	cd libutf     && make
	cd libcscript && make
	cd cmd        && make

test: cmd/charset-devolve
	cd cmd && make test

clean:
	cd libdevolve && make clean
	cd libutf     && make clean
	cd libcscript && make clean
	cd cmd        && make clean

.FORCE:

show-targets:
	@show-makefile-targets

show-%:
	@echo $*=$($*)
