CC=gcc

SDIR=src
ODIR=obj

PROG_DEPS_=prog.h
PROG_DEPS=$(patsubst %,$(SDIR)/%,$(PROG_DEPS_))

ASM_DEPS_=asm.h
ASM_DEPS=$(patsubst %,$(SDIR)/%,$(ASM_DEPS_))
ASM_OBJ_=prog.o asm_args.o asm_dir_ins.o asm.o
ASM_OBJ=$(patsubst %,$(ODIR)/%,$(ASM_OBJ_))

EMU_DEPS_=emu.h
EMU_DEPS=$(patsubst %,$(SDIR)/%,$(EMU_DEPS_))
EMU_OBJ_=prog.o emu_args.o emu_run.o emu.o
EMU_OBJ=$(patsubst %,$(ODIR)/%,$(EMU_OBJ_))

.PHONY: all
all: asm emu

asm: $(ASM_OBJ)
	mkdir -p $(ODIR)
	$(CC) -o $@ $^

emu: $(EMU_OBJ)
	mkdir -p $(ODIR)
	$(CC) -o $@ $^

.PHONY: example
example: example/prog_startup.s example/prog.s example/prog_d.s example/prog_num.s
	./asm -a 0 example/prog_startup.s example/prog_startup.o
	./asm -a 1024 example/prog.s example/prog.o
	./asm -a 2048 example/prog_d.s example/prog_d.o
	./asm -a 3072 example/prog_num.s example/prog_num.o
	./emu -b example/prog1 -l example/prog1.o example/prog.o example/prog_d.o example/prog_startup.o example/prog_num.o

$(ODIR)/asm.o: $(SDIR)/asm.c $(ASM_DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $<

$(ODIR)/asm_dir_ins.o: $(SDIR)/asm_dir_ins.c $(ASM_DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $<

$(ODIR)/asm_args.o: $(SDIR)/asm_args.c $(ASM_DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $<	

$(ODIR)/emu.o: $(SDIR)/emu.c $(EMU_DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $<

$(ODIR)/emu_run.o: $(SDIR)/emu_run.c $(EMU_DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $<

$(ODIR)/emu_args.o: $(SDIR)/emu_args.c $(EMU_DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $<

$(ODIR)/prog.o: $(SDIR)/prog.c $(PROG_DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $<

.PHONY: clean
clean:
	rm -rf $(ODIR)
