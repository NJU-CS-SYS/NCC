CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -std=c11 -Wall -Werror -MD -ggdb #-D DEBUG

GDBFLAGS = -ex "set args test.cmm test.S"\
		   -ex "set print pretty on"


LFILE = $(shell find ./ -name "*.l")
YFILE = $(shell find ./ -name "*.y")

LFC = $(shell find ./ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
YFC = $(shell find ./ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
CFILES = $(filter-out $(LFC) $(YFC), $(shell find ./ -name "*.c"))

OBJS = $(CFILES:.c=.o)
LFO = $(LFC:.c=.o)
YFO = $(YFC:.c=.o)

COMPILER := ncc

$(COMPILER): $(YFO) $(LFO) $(OBJS)
	$(CC) -ggdb -o $@ $^

$(LFO): $(LFC)
	$(CC) -ggdb -c $^

$(YFO): $(YFC)
	$(CC) -ggdb -c $^

$(LFC): $(LFILE)
	$(FLEX) -o $@ $^

$(YFC): $(YFILE)
	$(BISON) -o $@ -d -v $^

-include $(patsubst %.o, %.d, $(OBJS))

.PHONY: clean test gdb

test: $(COMPILER)
	./test.sh

execute: $(COMPILER)
	./test.sh execute

nemu: $(COMPILER)
	./test.sh nemu

gdb: $(COMPILER)
	gdb $(COMPILER) $(GDBFLAGS)

clean:
	rm -f $(COMPILER) syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h) $(LFO) $(YFO)
	rm -f *~
	rm -f ./test/*.out
	rm -f ./test/*.S
	rm -f ./test/*.debug
	rm -f ./test/*.ir
