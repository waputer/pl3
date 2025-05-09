-include make.conf

SRC = main.c

CHDR = cimpl/src/common.h \
       cimpl/src/config.h \
       cimpl/src/defs.h \
       cimpl/src/inc.h

CSRC = cimpl/src/main.c \
       cimpl/src/attr.c \
       cimpl/src/builtin.c \
       cimpl/src/check.c \
       cimpl/src/expr.c \
       cimpl/src/llvm.c \
       cimpl/src/parse.c \
       cimpl/src/scope.c \
       cimpl/src/stmt.c \
       cimpl/src/tag.c \
       cimpl/src/top.c \
       cimpl/src/type.c \
       cimpl/src/value.c

MSRC = misc/avl/avl.c \
       misc/mdbg/mdbg.c \
       misc/membuf/membuf.c \
       misc/lex/lex.c \
       misc/ifs/posix.c \
       misc/ifs/ifs.c \
       misc/isys/posix.c \
       misc/isys/isys.c \
       misc/bio/bio.c

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
       self/src/meta.el \
       self/src/parse.pl3 \
       self/src/stmt.pl3 \
       self/src/top.el \
       self/src/type.pl3

BIN = pl3-1.0.0-dev0
CFLAGS = -O2 -fPIC -DTEST -I.
LDFLAGS = -lgmp -lpthread

MOBJ = $(MSRC:.c=.o)
MDEP = $(MSRC:.c=.d)
MHDR = $(MSRC:.c=.h) misc/inline/inline.h misc/emech/emech.h

COBJ = $(CSRC:.c=.o)
CDEP = $(CSRC:.c=.d)
CHDR = $(CSRC:.c=.h) cimpl/src/common.h cimpl/src/config.h cimpl/src/defs.h cimpl/src/inc.h

# top level rules
all: $(BIN)

check: pl3-chk

run: test

clean:
	rm -f $(MOBJ) $(COBJ)

dist: $(BIN)-src.tar.xz

.PHONY: all check dist clean

# how to build the final binary
$(BIN): $(COBJ) $(MOBJ)
	gcc $(COBJ) $(MOBJ) $(LDFLAGS) -o $@

# the helper libraries header
misc.h: $(MHDR) Makefile
	echo "#pragma once" > $@
	tail -qn+2 $(MHDR) >> $@

test: $(BIN) test/basic.pl3
	./pl3_dev test/basic.pl3 $(RT) -o tmp.ll
	llc -relocation-model=pic -filetype=obj tmp.ll -o tmp.o
	gcc -O2 main.c tmp.o -o tmp
	./tmp

.PHONY: test

# building the distribution
$(BIN)-src.tar.xz: Makefile $(SRC) $(CSRC) $(CHDR) $(MSRC) $(MHDR) $(RT) $(SELF) LICENSE
	rm -rf $(BIN)-src
	mkdir $(BIN)-src
	cp -a --parents $^ $(BIN)-src
	tar -Jcf $@ $(BIN)-src

# rules for creating the checked compiler
pl3-self: pl3-self.o main.c
	gcc -O0 $^ -o $@

pl3-self.ll: $(BIN) $(RT) $(SELF) Makefile
	./$(BIN) $(RT) $(SELF) -o $@

# rules for rechecking the compiler (compling itself, again)
pl3-chk: pl3-chk.o main.c
	gcc -O2 $^ -o $@

pl3-chk.ll: pl3-self $(RT) $(SELF) Makefile
	./pl3-self $(RT) $(SELF) -o $@

# template rules
%.o: %.c misc.h Makefile
	gcc $(CFLAGS) -c $< -o $@

%.o: %.ll
	opt -O0 $< -S -o $<.opt
	llc -O0 -relocation-model=pic -filetype=obj $<.opt -o $@


.all: foo

.run: foo
	./foo

foo: all foo.pl3 $(RT) main.c Makefile
	./$(BIN) foo.pl3 $(RT) -o foo.ll
	llc -O1 -relocation-model=pic -filetype=obj foo.ll -o foo.o
	gcc $(CFLAGS) -O2 main.c foo.o -o foo
