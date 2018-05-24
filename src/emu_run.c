#include "emu.h"

#include <stdlib.h>
#include <string.h>

void emu_run(Program* prog)
{
    CPU* cpu = new_cpu();

    cpu_free(&cpu);
}

CPU* new_cpu()
{
    CPU* cpu = malloc(sizeof(CPU));
    int i;
    for (i = 0; i < 8; i++) cpu->reg[i] = 0;
    for (i = 0; i < 256; i++) cpu->mem[i] = 0;
    return cpu;
}

void cpu_free(CPU** cpu)
{
    int i;
    if (!(*cpu)) return;

    for (i = 0; i < 256; i++)
    {
        if ((*cpu)->mem[i])
        {
            free((*cpu)->mem[i]);
            (*cpu)->mem[i] = 0;
        }
    }
    free(*cpu);
    *cpu = 0;
}

int cpu_load_prog(CPU* cpu, Program* prog)
{
    SymbolTableNode* sym_node;
    DataListNode* data_node;
    if (!cpu || !prog) return 0;

    for (sym_node = prog->symbol_table_head; sym_node; sym_node = sym_node->next)
    {
        if (strcmp(sym_node->name, "START") == 0) break;
    }
    if (!sym_node) return 0;

    cpu->reg[7] = sym_node->offset;
    sym_node = prog->symbol_table_head;
    for (data_node = prog->data_head; data_node; data_node = data_node->next)
    {
        int i;
        while (sym_node->type != SYM_SECTION) sym_node = sym_node->next;

        for (i = 0; i < data_node->data_size; i++)
        {
            cpu_w(cpu, sym_node->offset + i, data_node->data_buffer[i]);
        }

        sym_node =sym_node->next;
    }
}

char cpu_r(CPU* cpu, int addr)
{
    if (!cpu || !cpu->mem || addr < 0 || addr >= 1 << 16) return -1;
    if (cpu->mem[addr >> 8]) return cpu->mem[addr >> 8][addr & 0xFF];
    return 0;
}

char cpu_w(CPU* cpu, int addr, char val)
{
    if (!cpu || !cpu->mem || addr < 0 || addr >= 1 << 16) return 0;
    if (!cpu->mem[addr >> 8]) cpu->mem[addr >> 8] = calloc(256, sizeof(char));
    cpu->mem[addr >> 8][addr & 0xFF] = val; 
    return 1;
}
