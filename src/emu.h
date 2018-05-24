#ifndef EMU_H
#define EMU_H

#include <argp.h>

#include "prog.h"

typedef enum { ARGS_VERB_SILENT, ARGS_VERB_NORMAL, ARGS_VERB_VERBOSE } ARGS_VERB;

typedef struct InputListNode
{
    char file[250];
    struct InputListNode* next;
} InputListNode;

typedef struct EmuArgs
{
    ARGS_VERB verb;
    InputListNode* in_head;
    InputListNode* in_tail;
    int do_link;
    char link_target[250];
    int do_build;
    char build_target[250];
    int do_run;
} EmuArgs;

typedef struct CPU
{
    int reg[8];
    int psw;
    char* mem[256];
} CPU;

void parse_args(int argc, char** argv, EmuArgs* args);
void emu_run(Program* prog);

CPU* new_cpu();
void cpu_free(CPU** cpu);
int cpu_load_prog(CPU* cpu, Program* prog);
char cpu_r(CPU* cpu, int addr);
char cpu_w(CPU* cpu, int addr, char val);

#endif  // EMU_H
