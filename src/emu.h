#ifndef EMU_H
#define EMU_H

#include <argp.h>
#include <stdint.h>

#include "prog.h"

typedef enum { ARGS_VERB_SILENT, ARGS_VERB_NORMAL, ARGS_VERB_VERBOSE } ARGS_VERB;

typedef enum { COND_EQ, COND_NE, COND_GT, COND_AL } INS_COND;
typedef enum 
{ 
    INS_ADD, INS_SUB, INS_MUL, INS_DIV, INS_CMP, INS_AND, INS_OR, INS_NOT, INS_TEST,
    INS_PUSH, INS_POP, INS_CALL, INS_IRET, INS_MOV, INS_SHL, INS_SHR 
} INSTRUCTION;
typedef enum { ADDR_PSW, ADDR_IMM, ADDR_REGDIR, ADDR_MEMDIR, ADDR_REGINDDISP } ADDRESSING;

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
    int16_t reg[8];
    int16_t psw;
    char* mem[256];
} CPU;

void parse_args(int argc, char** argv, EmuArgs* args);
void emu_run(Program* prog);

CPU* new_cpu();
void cpu_free(CPU** cpu);
int cpu_load_prog(CPU* cpu, Program* prog);

char cpu_r(CPU* cpu, uint16_t addr);
char cpu_w(CPU* cpu, uint16_t addr, char val);

int cpu_ri(CPU* cpu);
int cpu_rt(CPU* cpu);
int cpu_rn(CPU* cpu);
int cpu_rc(CPU* cpu);
int cpu_ro(CPU* cpu);
int cpu_rz(CPU* cpu);

void cpu_wi(CPU* cpu, int i);
void cpu_wt(CPU* cpu, int t);
void cpu_wn(CPU* cpu, int n);
void cpu_wc(CPU* cpu, int c);
void cpu_wo(CPU* cpu, int o);
void cpu_wz(CPU* cpu, int z);

#endif  // EMU_H
