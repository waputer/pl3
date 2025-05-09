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
       self/src/expr.pl3 \
       self/src/list.pl3 \
       self/src/llvm.pl3 \
       self/src/meta.pl3 \
       self/src/parse.pl3 \
       self/src/stmt.pl3 \
       self/src/top.pl3 \
       self/src/type.pl3

VER     := 1.0.0-dev1
PL3     := pl3
BIN     := pl3-$(VER)
CFLAGS  := -O2
LDFLAGS :=
LLC     := llc
OPT     := opt
OLVL    := -O0

all:

-include make.conf

# top level rules
all: $(BIN)

check: pl3-check

clean:
	rm $(BIN)

dist: pl3-$(VER)-src.tar.xz

# building the distribution
pl3-$(VER)-src.tar.xz: Makefile main.c $(RT) $(SELF) LICENSE
	rm -rf pl3-$(VER)-src
	mkdir pl3-$(VER)-src
	cp -a --parents $^ pl3-$(VER)-src
	tar -Jcf $@ pl3-$(VER)-src
	rm -rf pl3-$(VER)-src

# main build rules
$(BIN): pl3.o main.c
	gcc $^ -o $@

pl3.ll: $(RT) $(SELF) Makefile
	$(PL3) $(RT) $(SELF) -o $@

# rules for creating the self-built compiler
pl3-self: pl3-self.o main.c
	gcc -O1 $^ -o $@

pl3-self.ll: $(BIN) $(RT) $(SELF) Makefile
	./$(BIN) $(RT) $(SELF) -o $@

# rules for creating the checked compiler
pl3-check: pl3-check.o main.c
	gcc -O1 $^ -o $@

pl3-check.ll: pl3-self $(RT) $(SELF) Makefile
	./pl3-self $(RT) $(SELF) -o $@

# template rules
%.ll.opt: %.ll
	$(OPT) $(OLVL) $< -o $@

%.o: %.ll.opt
	$(LLC) $(OLVL) -filetype=obj -relocation-model=pic $< -o $@
