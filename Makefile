RTSRC = rt/src/avl/avl.pl3 \
        rt/src/os/linux/linux.pl3 \
        rt/src/os/linux/io.pl3 \
        rt/src/os/linux/mem.pl3 \
        rt/src/os/linux/poll.pl3 \
        rt/src/os/linux/sched.pl3 \
        rt/src/os/linux/tm.pl3 \
	rt/src/os/linux/thread.pl3 \
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
        rt/src/str/parse.pl3 \
        rt/src/str/std.pl3 \
        rt/src/math/flt.pl3 \
        rt/src/math/int.pl3 \
        rt/src/math/lim.pl3 \
        rt/src/mem/alloc.pl3\
        rt/src/mem/buf.pl3 \
        rt/src/mem/std.pl3 \
        rt/src/avl/test.pl3 \
        lib/src/dwarf.pl3 \
        lib/src/elf.pl3

LIBSRC = lib/src/check.pl3 \
         lib/src/enum.pl3 \
         lib/src/decl.pl3 \
         lib/src/expr.pl3 \
         lib/src/io.pl3 \
         lib/src/list.pl3 \
         lib/src/llvm.pl3 \
         lib/src/meta.pl3 \
         lib/src/parse.pl3 \
         lib/src/sig.pl3 \
         lib/src/stmt.pl3 \
         lib/src/top.pl3 \
         lib/src/type.pl3

BINSRC = bin/src/run.pl3

PL3 := ../bin/pl3
VER := 1.0.0-dev3
BIN := pl3-$(VER)

## standard build

all: $(BIN)

rt.o: $(RTSRC) Makefile
	$(PL3) $(RTSRC) -o $(@:.o=.ll) -e rt.pl3.mod
	llc -O0 rt.ll -o $@ -filetype=obj -relocation-model=pic

lib.o: $(LIBSRC) Makefile rt.o
	$(PL3) $(LIBSRC) -o $(@:.o=.ll) -i rt.pl3.mod -e lib.pl3.mod
	llc -O0 lib.ll -o $@ -filetype=obj -relocation-model=pic

bin.o: $(BINSRC) Makefile rt.o lib.o
	$(PL3) $(BINSRC) -o $(@:.o=.ll) -i rt.pl3.mod -i lib.pl3.mod
	llc -O0 $(@:.o=.ll) -o $@ -filetype=obj -relocation-model=pic

$(BIN): bin.o lib.o rt.o start.c
	gcc $^ -o $@

## compiler check build

check: pl3rechk

rtchk.o: $(RTSRC) Makefile $(BIN)
	./$(BIN) $(RTSRC) -o $(@:.o=.ll) -e $(@:.o=.pl3.mod)
	llc -O0 $(@:.o=.ll) -o $@ -filetype=obj -relocation-model=pic

libchk.o: $(LIBSRC) Makefile rtchk.o $(BIN)
	./$(BIN) $(LIBSRC) -o $(@:.o=.ll) -i rtchk.pl3.mod -e $(@:.o=.pl3.mod)
	llc -O0 $(@:.o=.ll) -o $@ -filetype=obj -relocation-model=pic

binchk.o: $(BINSRC) Makefile rtchk.o libchk.o $(BIN)
	./$(BIN) $(BINSRC) -o $(@:.o=.ll) -i rtchk.pl3.mod -i libchk.pl3.mod
	llc -O0 $(@:.o=.ll) -o $@ -filetype=obj -relocation-model=pic

pl3chk: binchk.o libchk.o rtchk.o start.c
	gcc $^ -o $@

rtrechk.o: $(RTSRC) Makefile pl3chk
	./pl3chk $(RTSRC) -o $(@:.o=.ll) -e $(@:.o=.pl3.mod)
	llc -O0 $(@:.o=.ll) -o $@ -filetype=obj -relocation-model=pic

librechk.o: $(LIBSRC) Makefile rtrechk.o pl3chk
	./pl3chk $(LIBSRC) -o $(@:.o=.ll) -i rtrechk.pl3.mod -e $(@:.o=.pl3.mod)
	llc -O0 $(@:.o=.ll) -o $@ -filetype=obj -relocation-model=pic

binrechk.o: $(BINSRC) Makefile rtrechk.o librechk.o pl3chk
	./pl3chk $(BINSRC) -o $(@:.o=.ll) -i rtrechk.pl3.mod -i librechk.pl3.mod
	llc -O0 $(@:.o=.ll) -o $@ -filetype=obj -relocation-model=pic

pl3rechk: binrechk.o librechk.o rtrechk.o start.c
	gcc $^ -o $@

## cleanup

clean:
	rm -f *.o pl3 pl3chk pl3rechk

# distribute

dist: pl3-$(VER)-src.tar.xz

pl3-$(VER)-src.tar.xz: Makefile start.c $(RTSRC) $(LIBSRC) $(BINSRC) LICENSE
	rm -rf pl3-$(VER)-src
	mkdir pl3-$(VER)-src
	cp -a --parents $^ pl3-$(VER)-src
	tar -Jcf $@ pl3-$(VER)-src
	rm -rf pl3-$(VER)-src

#-include user.mk
