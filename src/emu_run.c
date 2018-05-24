#include "emu.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void emu_run(Program* prog)
{
    CPU* cpu = new_cpu();
    cpu_load_prog(cpu, prog);

    while (1)
    {
        char ins_c1, ins_c2, arg1_code, arg2_code;
        int16_t arg1_imm = 0, arg2_imm = 0;
        int16_t arg1_val = 0, arg2_val = 0, res_val = 0;
        char set_n = 0, set_c = 0, set_o = 0, set_z = 0, store_res = 0;
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

        if (ins != INS_IRET)
        {
            arg1_code = ((ins_c1 & 0b11) << 3) | (ins_c2 >> 5);
            switch (arg1_code >> 3)
            {
            case 0b00: addr1 = ( (arg1_code & 0b111) == 0b111 ? ADDR_PSW : ADDR_IMM ); break;
            case 0b01: addr1 = ADDR_REGDIR; break;
            case 0b10: addr1 = ADDR_MEMDIR; break;
            case 0b11: addr1 = ADDR_REGINDDISP; break;
            }
            if (addr1 != ADDR_REGDIR)
            {
                arg1_imm = cpu_r(cpu, cpu->reg[7]) << 8 + cpu_r(cpu, cpu->reg[7] + 1);
                cpu->reg[7] += 2;
            }
            switch (addr1)
            {
            case ADDR_PSW: arg1_val = cpu->psw; break;
            case ADDR_IMM: arg1_val = arg1_imm; break;
            case ADDR_REGDIR: arg1_val = cpu->reg[arg1_code & 0b111]; break;
            case ADDR_MEMDIR: arg1_val = cpu_r(cpu, arg1_imm) << 8 + cpu_r(cpu, arg1_imm + 1); break;
            case ADDR_REGINDDISP: 
                arg1_val = 
                    cpu_r(cpu, arg1_imm + cpu->reg[arg1_code & 0b111]) << 8 + 
                    cpu_r(cpu, arg1_imm + cpu->reg[arg1_code & 0b111] + 1); 
                break;
            }

            if (ins != INS_PUSH && ins == INS_POP && ins == INS_CALL)
            {
                arg2_code = ins_c2 & 0b11111;
                switch (arg2_code >> 3)
                {
                case 0b00: addr2 = ( (arg2_code & 0b111) == 0b111 ? ADDR_PSW : ADDR_IMM ); break;
                case 0b01: addr2 = ADDR_REGDIR; break;
                case 0b10: addr2 = ADDR_MEMDIR; break;
                case 0b11: addr2 = ADDR_REGINDDISP; break;
                }
                if (addr2 != ADDR_REGDIR)
                {
                    if (addr1 != ADDR_REGDIR) return;
                    arg2_imm = cpu_r(cpu, cpu->reg[7]) << 8 + cpu_r(cpu, cpu->reg[7] + 1);
                    cpu->reg[7] += 2;
                }
                switch (addr2)
                {
                case ADDR_PSW: arg2_val = cpu->psw; break;
                case ADDR_IMM: arg2_val = arg2_imm; break;
                case ADDR_REGDIR: arg2_val = cpu->reg[arg2_code & 0b111]; break;
                case ADDR_MEMDIR: arg2_val = cpu_r(cpu, arg2_imm) << 8 + cpu_r(cpu, arg2_imm + 1); break;
                case ADDR_REGINDDISP: 
                    arg2_val = 
                        cpu_r(cpu, arg2_imm + cpu->reg[arg2_code & 0b111]) << 8 + 
                        cpu_r(cpu, arg2_imm + cpu->reg[arg2_code & 0b111] + 1); 
                    break;
                }
            }
        }

        if (cond == COND_EQ && !cpu_rz(cpu)) continue;
        if (cond == COND_NE && !cpu_rn(cpu)) continue;
        if (cond == COND_GT && (cpu_rn(cpu) || cpu_rz(cpu))) continue;

        switch (ins)
        {
        case INS_ADD: 
            res_val = arg1_val + arg2_val;
            set_n = 1; set_c = 1; set_o = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_SUB:
            res_val = arg1_val - arg2_val;
            set_n = 1; set_c = 1; set_o = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_MUL:
            res_val = arg1_val * arg2_val;
            set_n = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_DIV:
            res_val = arg1_val * arg2_val;
            set_n = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_CMP:
            res_val = arg1_val - arg2_val;
            set_n = 1; set_c = 1; set_o = 1; set_z = 1;
            break;
        case INS_AND:
            res_val = arg1_val & arg2_val;
            set_n = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_OR:
            res_val = arg1_val | arg2_val;
            set_n = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_NOT:
            res_val = ~arg2_val;
            set_n = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_TEST:
            res_val = arg1_val & arg2_val;
            set_n = 1; set_z = 1;
            break;
        case INS_PUSH:
            cpu->reg[6] -= 2;
            cpu_w(cpu, cpu->reg[6], arg1_val >> 8);
            cpu_w(cpu, cpu->reg[6] + 1, arg1_val & 0xFF);
            break;
        case INS_POP:
            res_val = cpu_r(cpu, cpu->reg[6]++) << 8;
            res_val |= cpu_r(cpu, cpu->reg[6]++);
            store_res = 1;
            break;
        case INS_CALL:
            cpu->reg[6] -= 2;
            cpu_w(cpu, cpu->reg[6], cpu->reg[7] >> 8);
            cpu_w(cpu, cpu->reg[6] + 1, cpu->reg[7] & 0xFF);
            cpu->reg[7] = arg1_val;
            break;
        case INS_IRET:
            cpu->psw = cpu_r(cpu, cpu->reg[6]++) << 8;
            cpu->psw |= cpu_r(cpu, cpu->reg[6]++);
            cpu->reg[7] = cpu_r(cpu, cpu->reg[6]++) << 8;
            cpu->reg[7] |= cpu_r(cpu, cpu->reg[6]++);
            break;
        case INS_MOV:
            res_val = arg2_val;
            set_n = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_SHL:
            res_val = arg1_val << arg2_val;
            set_n = 1; set_c = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_SHR:
            res_val = arg1_val >> arg2_val;
            set_n = 1; set_c = 1; set_z = 1;
            store_res = 1;
            break;
        }

        if (set_n)
        {
            if (res_val < 0) cpu_wn(cpu, 1);
            else cpu_wn(cpu, 0);
        }
        if (set_c)
        {

        }
        if (set_o)
        {

        }
        if (set_z)
        {
            if (res_val = 0) cpu_wz(cpu, 1);
            else cpu_wz(cpu, 0);
        }

        if (store_res)
        {
            switch (addr1)
            {
            case ADDR_PSW: cpu->psw = res_val; break;
            case ADDR_IMM: return; break;
            case ADDR_REGDIR: cpu->reg[arg1_code & 0b111] = res_val; break;
            case ADDR_MEMDIR: 
                cpu_w(cpu, arg1_imm, res_val >> 8);
                cpu_w(cpu, arg1_imm + 1, res_val & 0xFF);
                break;
            case ADDR_REGINDDISP:
                cpu_w(cpu, arg1_imm + cpu->reg[arg1_code & 0b111], res_val >> 8);
                cpu_w(cpu, arg1_imm + cpu->reg[arg1_code & 0b111] + 1, res_val & 0xFF);
                break;
            }
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
    cpu->psw = 0;
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

    cpu->reg[6] = (1 << 16) - 128;
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

int cpu_rn(CPU* cpu)
{
    if (cpu->psw & 0b1000) return 1;
    return 0;
}

int cpu_rc(CPU* cpu)
{
    if (cpu->psw & 0b100) return 1;
    return 0;
}

int cpu_ro(CPU* cpu)
{
    if (cpu->psw & 0b10) return 1;
    return 0;
}

int cpu_rz(CPU* cpu)
{
    if (cpu->psw & 0b1) return 1;
    return 0;
}

void cpu_wn(CPU* cpu, int n)
{
    if (n) cpu->psw |= 0b1000;
    else cpu->psw &= ~0b1000;
}

void cpu_wc(CPU* cpu, int c)
{
    if (c) cpu->psw |= 0b100;
    else cpu->psw &= ~0b100;
}

void cpu_wo(CPU* cpu, int o)
{
    if (o) cpu->psw |= 0b10;
    else cpu->psw &= ~0b10;
}

void cpu_wz(CPU* cpu, int z)
{
    if (z) cpu->psw |= 0b1;
    else cpu->psw &= ~0b1;
}