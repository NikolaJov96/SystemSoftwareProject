#include "emu.h"

#include <stdlib.h>
#include <string.h>

void emu_run(Program* prog)
{
    CPU* cpu = new_cpu();
    cpu_load_prog(cpu, prog);

    while (1)
    {
        char ins_c1, ins_c2, arg1, arg2;
        INS_COND cond;
        INSTRUCTION ins;
        ADDRESSING addr1, addr2;

        ins_c1 = cpu_r(cpu, cpu->reg[7]++);
        ins_c2 = cpu_r(cpu, cpu->reg[7]++);
        
        switch (ins_c1 >> 6)
        {
        case 0b00: cond = COND_EQ; break;
        case 0b01: cond = COND_NE; break;
        case 0b10: cond = COND_GT; break;
        case 0b11: cond = COND_AL; break;
        }

        switch ((ins_c1 >> 2) & 0xFF)
        {
        case 0b0000: ins = INS_ADD; break;
        case 0b0001: ins = INS_SUB; break;
        case 0b0010: ins = INS_MUL; break;
        case 0b0011: ins = INS_DIV; break;
        case 0b0100: ins = INS_CMP; break;
        case 0b0101: ins = INS_AND; break;
        case 0b0110: ins = INS_OR; break;
        case 0b0111: ins = INS_NOT; break;
        case 0b1000: ins = INS_TEST; break;
        case 0b1001: ins = INS_PUSH; break;
        case 0b1010: ins = INS_POP; break;
        case 0b1011: ins = INS_CALL; break;
        case 0b1100: ins = INS_IRET; break;
        case 0b1101: ins = INS_MOV; break;
        case 0b1110: ins = INS_SHL; break;
        case 0b1111: ins = INS_SHR; break;
        }

        arg1 = ((ins_c1 & 0b11) << 3) | (ins_c2 >> 5);
        arg2 = ins_c2 & 0b11111;
        
        switch (arg1 >> 3)
        {
        case 0b00: addr1 = ( (arg1 & 0b111) == 0b111 ? ADDR_PSW : ADDR_IMM ); break;
        case 0b01: addr1 = ADDR_REGDIR; break;
        case 0b10: addr1 = ADDR_MEMDIR; break;
        case 0b11: addr1 = ADDR_REGINDDISP; break;
        }

        switch (arg2 >> 3)
        {
        case 0b00: addr2 = ( (arg2 & 0b111) == 0b111 ? ADDR_PSW : ADDR_IMM ); break;
        case 0b01: addr2 = ADDR_REGDIR; break;
        case 0b10: addr2 = ADDR_MEMDIR; break;
        case 0b11: addr2 = ADDR_REGINDDISP; break;
        }

        break;
    }

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
