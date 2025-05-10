RT = rt/src/avl/avl.pl3 \
     rt/src/os/linux/linux.pl3 \
     rt/src/os/linux/io.pl3 \
     rt/src/os/linux/mem.pl3 \
     rt/src/os/linux/tm.pl3 \
     rt/src/io/accum.pl3 \
     rt/src/io/buf.pl3 \
     rt/src/io/file.pl3 \
     rt/src/io/fmt.pl3 \
     rt/src/io/view.pl3 \
     rt/src/lex/ch.pl3 \
     rt/src/lex/parse.pl3 \
     rt/src/lex/rules.pl3 \
     rt/src/str/buf.pl3 \
     rt/src/str/fmt.pl3 \
     rt/src/str/std.pl3 \
     rt/src/math/int.pl3 \
     rt/src/math/lim.pl3 \
     rt/src/mem/alloc.pl3\
     rt/src/mem/buf.pl3 \
     rt/src/mem/std.pl3 \
     rt/src/avl/test.pl3 \
     self/src/dwarf.pl3 \
     self/src/elf.pl3

SELF = self/src/check.pl3 \
       self/src/enum.pl3 \
       self/src/decl.pl3 \
       self/src/expr.pl3 \
       self/src/io.pl3 \
       self/src/list.pl3 \
       self/src/llvm.pl3 \
       self/src/meta.pl3 \
       self/src/parse.pl3 \
       self/src/sig.pl3 \
       self/src/stmt.pl3 \
       self/src/top.pl3 \
       self/src/type.pl3

VER     := 1.0.0-dev2
PL3     := pl3
RTOBJ   := rt.o
RTMOD   := rt.pl3.mod
BIN     := pl3-$(VER)
CFLAGS  := -O2
LDFLAGS :=
LLC     := llc
OPT     := opt
OLVL    := -O0

all:

-include make.conf

# top level rules
all: $(BIN) rt.all start.o

check: pl3-check

clean:
	rm $(BIN)

dist: pl3-$(VER)-src.tar.xz

# building the distribution
pl3-$(VER)-src.tar.xz: Makefile main.c start.c $(RT) $(SELF) self/src/run.pl3 LICENSE
	rm -rf pl3-$(VER)-src
	mkdir pl3-$(VER)-src
	cp -a --parents $^ pl3-$(VER)-src
	tar -Jcf $@ pl3-$(VER)-src
	rm -rf pl3-$(VER)-src

# main build rules
$(BIN): pl3.o main.c
	gcc -g $^ -o $@

pl3.ll: $(RT) $(SELF) self/src/run.pl3 Makefile
	$(PL3) $(RT) $(SELF) self/src/run.pl3 -o $@

.PHONY: rt.all
rt.all: $(RT) Makefile
	./$(BIN) $(RT) -o rt.ll -e rt.pl3.mod
	llc -O0 -filetype=obj -relocation-model=pic rt.ll -o rt.o

rt.o rt.pl3.mod: rt.all

start.o: start.c Makefile
	gcc -c -O2 -fPIC start.c -o start.o

# rules for creating the self-built compiler
pl3-self: pl3-self.o main.c
	gcc -O1 $^ -o $@

pl3-self.ll: $(BIN) $(RT) $(SELF) self/src/run.pl3 Makefile
	./$(BIN) $(RT) $(SELF) self/src/run.pl3 -o $@

# rules for creating the checked compiler
pl3-check: pl3-check.o main.c
	gcc -O1 $^ -o $@

pl3-check.ll: pl3-self $(RT) $(SELF) self/src/run.pl3 Makefile
	./pl3-self $(RT) $(SELF) self/src/run.pl3 -o $@

# template rules
%.ll.opt: %.ll
	$(OPT) $(OLVL) $< -o $@

%.o: %.ll
	$(LLC) $(OLVL) -filetype=obj -relocation-model=pic $< -o $@
