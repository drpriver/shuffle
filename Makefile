Bin: ; mkdir -p $@
Deps: ; mkdir -p $@
DEBUG=
OPT=-O3

DEPFILES:= $(wildcard Deps/*.dep)
include $(DEPFILES)

Bin/shuffle: shuffle.c | Deps Bin
	$(CC) $< -o $@ -MT $@ -MD -MP -MF Deps/shuffle.dep $(OPT) $(DEBUG)

README.html: README.md README.css
	pandoc README.md README.css -f markdown -o $@ -s --toc

.PHONY: clean
clean:
	rm -rf Bin/*
.PHONY: all
all: Bin/shuffle

.DEFAULT_GOAL:=all
