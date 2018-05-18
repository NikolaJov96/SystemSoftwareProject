CC=gcc

SDIR=src
ODIR=obj

PROG_DEPS_=prog.h
PROG_DEPS=$(patsubst %,$(SDIR)/%,$(PROG_DEPS_))

ASM_DEPS_=asm.h
ASM_DEPS=$(patsubst %,$(SDIR)/%,$(ASM_DEPS_))
ASM_OBJ_=prog.o asm_args.o asm.o
ASM_OBJ=$(patsubst %,$(ODIR)/%,$(ASM_OBJ_))

EMU_DEPS_=
EMU_DEPS=$(patsubst %,$(SDIR)/%,$(EMU_DEPS_))
EMU_OBJ_=prog.o emu.o
EMU_OBJ=$(patsubst %,$(ODIR)/%,$(EMU_OBJ_))

.PHONY: all
all: asm emu

asm: $(ASM_OBJ)
	mkdir -p $(ODIR)
	$(CC) -o $@ $^

emu: $(EMU_OBJ)
	mkdir -p $(ODIR)
	$(CC) -o $@ $^

$(ODIR)/asm.o: $(SDIR)/asm.c $(ASM_DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $<

$(ODIR)/asm_args.o: $(SDIR)/asm_args.c $(ASM_DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $<	

$(ODIR)/emu.o: $(SDIR)/emu.c $(EMU_DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $<

$(ODIR)/prog.o: $(SDIR)/prog.c $(PROG_DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $<

.PHONY: clean
clean:
	rm -rf $(ODIR)
