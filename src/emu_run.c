#include "emu.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

void emu_run(Program* prog, int verbose)
{
    CPU* cpu = new_cpu();
    clock_t last_time, curr_time;struct termios oldt, newt;
    int ch;
    int oldf;

    switch (cpu_load_prog(cpu, prog))
    {
    case 1: if (verbose) printf("Fatal error.\n"); return; break;
    case 2: if (verbose) printf("Start symbol not defined.\n"); return; break;
    case 3: if (verbose) printf("Cannot find start symbol section.\n"); return; break;
    case 4: if (verbose) printf("There are unresolved relocations in the program.\n"); return; break;
    }

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    if (cpu_ri(cpu))
    {
        uint16_t addr = (cpu_r(cpu, IVT_STARTUP) & 0xFF) | (cpu_r(cpu, IVT_STARTUP + 1) << 8);
        if (addr != 0)
        {
            cpu->reg[6] -= 2;
            cpu_w(cpu, cpu->reg[6], cpu->reg[7] & 0xFF);
            cpu_w(cpu, cpu->reg[6] + 1, cpu->reg[7] >> 8);
            cpu->reg[6] -= 2;
            cpu_w(cpu, cpu->reg[6], cpu->psw & 0xFF);
            cpu_w(cpu, cpu->reg[6] + 1, cpu->psw >> 8);
            cpu->reg[7] = addr;
        }
    }

    last_time = clock();
    while (1)
    {
        unsigned char ins_c1, ins_c2, arg1_code, arg2_code;
        uint16_t arg1_imm = 0, arg2_imm = 0;
        int16_t arg1_val = 0, arg2_val = 0, res_val = 0;
        char set_n = 0, set_z = 0, store_res = 0;
        char printf_char, input_char;
        INS_COND cond;
        INSTRUCTION ins;
        ADDRESSING addr1, addr2;

        if (cpu_rd(cpu)){
            int i;
            printf("(regs: ");
            for (i = 0; i < 8; i++) printf("%d ", cpu->reg[i]);
            printf(" i:%d d:%d t:%d n:%d c:%d o:%d z:%d  ins: %02hhx %02hhx)\n", 
                cpu_ri(cpu), cpu_rd(cpu), cpu_rt(cpu), cpu_rn(cpu), cpu_rc(cpu), cpu_ro(cpu), cpu_rz(cpu),
                cpu_r(cpu, cpu->reg[7]), cpu_r(cpu, cpu->reg[7] + 1));
        }

        printf_char = cpu_r(cpu, 0xFFFE);
        if (printf_char)
        {
            if (printf_char == 0x10) printf("\n");
            else
            {
                printf("%c", printf_char);
                fflush(stdout);
            }
            cpu_w(cpu, 0xFFFE, 0);
            cpu_w(cpu, 0xFFFF, 0);
        }

        input_char = ch = getchar();
        if (input_char != EOF)
        {
            if (cpu_ri(cpu))
            {
                cpu_w(cpu, 0xFFFC, input_char);
                cpu_w(cpu, 0xFFFD, 0);
                uint16_t addr = (cpu_r(cpu, IVT_KEY_PRESS) & 0xFF) | (cpu_r(cpu, IVT_KEY_PRESS + 1) << 8);
                if (addr != 0)
                {
                    cpu->reg[6] -= 2;
                    cpu_w(cpu, cpu->reg[6], cpu->reg[7] & 0xFF);
                    cpu_w(cpu, cpu->reg[6] + 1, cpu->reg[7] >> 8);
                    cpu->reg[6] -= 2;
                    cpu_w(cpu, cpu->reg[6], cpu->psw & 0xFF);
                    cpu_w(cpu, cpu->reg[6] + 1, cpu->psw >> 8);
                    cpu->reg[7] = addr;
                    continue;
                }
            }
        }

        curr_time = clock();
        if (curr_time - last_time > CLOCKS_PER_SEC)
        {
            last_time = curr_time;
            if (cpu_ri(cpu))
            {
                uint16_t addr = (cpu_r(cpu, IVT_TIMER) & 0xFF) | (cpu_r(cpu, IVT_TIMER + 1) << 8);
                if (addr != 0)
                {
                    cpu->reg[6] -= 2;
                    cpu_w(cpu, cpu->reg[6], cpu->reg[7] & 0xFF);
                    cpu_w(cpu, cpu->reg[6] + 1, cpu->reg[7] >> 8);
                    cpu->reg[6] -= 2;
                    cpu_w(cpu, cpu->reg[6], cpu->psw & 0xFF);
                    cpu_w(cpu, cpu->reg[6] + 1, cpu->psw >> 8);
                    cpu->reg[7] = addr;
                    continue;
                }
            }
        }

        ins_c1 = cpu_r(cpu, cpu->reg[7]++);
        ins_c2 = cpu_r(cpu, cpu->reg[7]++);
        if (ins_c1 == 0 && ins_c2 == 0) break;
        
        switch (ins_c1 >> 6)
        {
        case 0b00: cond = COND_EQ; break;
        case 0b01: cond = COND_NE; break;
        case 0b10: cond = COND_GT; break;
        case 0b11: cond = COND_AL; break;
        }

        switch ((ins_c1 >> 2) & 0xF)
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
        default:
            // interrupt
            break;
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
            if (addr1 != ADDR_REGDIR && addr1 != ADDR_PSW)
            {
                arg1_imm = (cpu_r(cpu, cpu->reg[7]) & 0xFF) + (cpu_r(cpu, cpu->reg[7] + 1) << 8);
                cpu->reg[7] += 2;
            }
            switch (addr1)
            {
            case ADDR_PSW: arg1_val = cpu->psw; break;
            case ADDR_IMM: arg1_val = arg1_imm; break;
            case ADDR_REGDIR: arg1_val = cpu->reg[arg1_code & 0b111]; break;
            case ADDR_MEMDIR: arg1_val = (cpu_r(cpu, arg1_imm) & 0xFF) + (cpu_r(cpu, arg1_imm + 1) << 8); break;
            case ADDR_REGINDDISP:
                arg1_imm += cpu->reg[arg1_code & 0b111];
                arg1_val = (cpu_r(cpu, arg1_imm) & 0xFF) + (cpu_r(cpu, arg1_imm + 1) << 8);
                break;
            }

            if (ins != INS_PUSH && ins != INS_POP && ins != INS_CALL)
            {
                arg2_code = ins_c2 & 0b11111;
                switch (arg2_code >> 3)
                {
                case 0b00: addr2 = ( (arg2_code & 0b111) == 0b111 ? ADDR_PSW : ADDR_IMM ); break;
                case 0b01: addr2 = ADDR_REGDIR; break;
                case 0b10: addr2 = ADDR_MEMDIR; break;
                case 0b11: addr2 = ADDR_REGINDDISP; break;
                }
                if (addr2 != ADDR_REGDIR && addr2 != ADDR_PSW)
                {
                    if (addr1 != ADDR_REGDIR && addr1 != ADDR_PSW) /* interrupt */ break;
                    arg2_imm = (cpu_r(cpu, cpu->reg[7]) & 0xFF) + (cpu_r(cpu, cpu->reg[7] + 1) << 8);
                    cpu->reg[7] += 2;

                    if ((addr1 == ADDR_REGDIR || addr1 == ADDR_REGINDDISP) && (arg1_code & 0b111 == 7))
                        arg1_val = cpu->reg[arg1_code & 0b111];
                }
                switch (addr2)
                {
                case ADDR_PSW: arg2_val = cpu->psw; break;
                case ADDR_IMM: arg2_val = arg2_imm; break;
                case ADDR_REGDIR: arg2_val = cpu->reg[arg2_code & 0b111]; break;
                case ADDR_MEMDIR: arg2_val = (cpu_r(cpu, arg2_imm) & 0xFF) + (cpu_r(cpu, arg2_imm + 1) << 8); break;
                case ADDR_REGINDDISP:
                    arg2_imm += cpu->reg[arg2_code & 0b111];
                    arg2_val = (cpu_r(cpu, arg2_imm) & 0xFF) + (cpu_r(cpu, arg2_imm + 1) << 8); 
                    break;
                }
            }
        }

        if (cond == COND_EQ && !cpu_rz(cpu)) continue;
        if (cond == COND_NE && cpu_rz(cpu)) continue;
        if (cond == COND_GT && (cpu_rn(cpu) || cpu_rz(cpu))) continue;

        switch (ins)
        {
        case INS_ADD:
            res_val = arg1_val + arg2_val;
            if ((arg1_val > 0 && arg2_val > 0 && res_val < 0) ||
                (arg1_val < 0 && arg2_val < 0 && res_val > 0)) cpu_wo(cpu, 1);
            else cpu_wo(cpu, 0);
            if (((arg1_val > 0 && arg2_val < 0) || (arg1_val < 0 && arg2_val > 0)) && res_val >= 0) cpu_wc(cpu, 1);
            else if (arg1_val < 0 && arg2_val < 0) cpu_wc(cpu, 1);
            else cpu_wc(cpu, 0);
            set_n = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_SUB:
            res_val = arg1_val - arg2_val;
            if (arg2_val == 0b10000000) cpu_wo(cpu, 1);
            else if ((arg1_val > 0 && arg2_val < 0 && res_val < 0) ||
                (arg1_val < 0 && arg2_val > 0 && res_val > 0)) cpu_wo(cpu, 1);
            else cpu_wo(cpu, 0);
            if (((arg1_val > 0 && -arg2_val < 0) || (arg1_val < 0 && -arg2_val > 0)) && res_val >= 0) cpu_wc(cpu, 1);
            else if (arg1_val < 0 && -arg2_val < 0) cpu_wc(cpu, 1);
            else cpu_wc(cpu, 0);
            set_n = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_MUL:
            res_val = arg1_val * arg2_val;
            set_n = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_DIV:
            res_val = arg1_val / arg2_val;
            set_n = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_CMP:
            res_val = arg1_val - arg2_val;
            if (arg2_val == 0b10000000) cpu_wo(cpu, 1);
            else if ((arg1_val > 0 && arg2_val < 0 && res_val < 0) ||
                (arg1_val < 0 && arg2_val > 0 && res_val > 0)) cpu_wo(cpu, 1);
            else cpu_wo(cpu, 0);
            if (((arg1_val > 0 && -arg2_val < 0) || (arg1_val < 0 && -arg2_val > 0)) && res_val >= 0) cpu_wc(cpu, 1);
            else if (arg1_val < 0 && -arg2_val < 0) cpu_wc(cpu, 1);
            else cpu_wc(cpu, 0);
            set_n = 1; set_z = 1;
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
            cpu_w(cpu, cpu->reg[6], arg1_val & 0xFF);
            cpu_w(cpu, cpu->reg[6] + 1, arg1_val >> 8);
            break;
        case INS_POP:
            res_val = cpu_r(cpu, cpu->reg[6]) & 0xFF;
            res_val |= cpu_r(cpu, cpu->reg[6] + 1) << 8;
            cpu->reg[6] += 2;
            store_res = 1;
            break;
        case INS_CALL:
            cpu->reg[6] -= 2;
            cpu_w(cpu, cpu->reg[6], cpu->reg[7] & 0xFF);
            cpu_w(cpu, cpu->reg[6] + 1, cpu->reg[7] >> 8);
            cpu->reg[7] = arg1_imm;
            break;
        case INS_IRET:
            cpu->psw = cpu_r(cpu, cpu->reg[6]++) & 0xFF;
            cpu->psw |= cpu_r(cpu, cpu->reg[6]++) << 8;
            cpu->reg[7] = cpu_r(cpu, cpu->reg[6]++) & 0xFF;
            cpu->reg[7] |= cpu_r(cpu, cpu->reg[6]++) << 8;
            break;
        case INS_MOV:
            res_val = arg2_val;
            set_n = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_SHL:
            res_val = arg1_val << arg2_val;
            if (arg2_val == 0 || arg2_val > 15) cpu_wc(cpu, 0);
            else if (arg1_val & (1 << (16 - arg2_val))) cpu_wc(cpu, 1);
            else cpu_wc(cpu, 0);
            set_n = 1; set_z = 1;
            store_res = 1;
            break;
        case INS_SHR:
            res_val = arg1_val >> arg2_val;
            if (arg2_val == 0 || arg2_val > 15) cpu_wc(cpu, 0);
            else if (arg1_val & (1 << (arg2_val - 1))) cpu_wc(cpu, 1);
            else cpu_wc(cpu, 0);
            set_n = 1; set_z = 1;
            store_res = 1;
            break;
        }

        if (set_n)
        {
            if (res_val < 0) cpu_wn(cpu, 1);
            else cpu_wn(cpu, 0);
        }
        if (set_z)
        {
            if (res_val == 0) cpu_wz(cpu, 1);
            else cpu_wz(cpu, 0);
        }

        if (store_res)
        {
            if (addr1 == ADDR_IMM) break;
            switch (addr1)
            {
            case ADDR_PSW: cpu->psw = res_val; break;
            case ADDR_IMM: break;
            case ADDR_REGDIR: cpu->reg[arg1_code & 0b111] = res_val; break;
            case ADDR_MEMDIR:
                cpu_w(cpu, arg1_imm, res_val & 0xFF);
                cpu_w(cpu, arg1_imm + 1, res_val >> 8);
                break;
            case ADDR_REGINDDISP:
                cpu_w(cpu, arg1_imm + cpu->reg[arg1_code & 0b111], res_val & 0xFF);
                cpu_w(cpu, arg1_imm + cpu->reg[arg1_code & 0b111] + 1, res_val >> 8);
                break;
            }
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

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
    SymbolTableNode* sec_node;
    DataListNode* data_node;
    if (!cpu || !prog) return 1;

    for (sym_node = prog->symbol_table_head; sym_node; sym_node = sym_node->next)
    {
        if (strcmp(sym_node->name, "start") == 0) break;
    }
    if (!sym_node) return 2;

    for (sec_node = prog->symbol_table_head; sec_node; sec_node = sec_node->next)
    {
        if (sec_node->sym_id == sym_node->section_id) break;
    }
    if (!sec_node) return 3;

    cpu->reg[6] = (1 << 16) - 128;
    cpu->reg[7] = sec_node->offset + sym_node->offset;
    cpu_wi(cpu, 1);
    sym_node = prog->symbol_table_head;
    for (data_node = prog->data_head; data_node; data_node = data_node->next)
    {
        int i;
        RelListNode* rel_node;
        if (data_node->rel_head) return 4;

        while (sym_node->type != SYM_SECTION) sym_node = sym_node->next;

        for (i = 0; i < data_node->data_size; i++)
        {
            cpu_w(cpu, sym_node->offset + i, data_node->data_buffer[i]);
        }

        sym_node =sym_node->next;
    }

    return 0;
}

char cpu_r(CPU* cpu, uint16_t addr)
{
    if (!cpu || !cpu->mem) return -1;
    if (cpu->mem[addr >> 8]) return cpu->mem[addr >> 8][addr & 0xFF];
    return 0;
}

char cpu_w(CPU* cpu, uint16_t addr, char val)
{
    if (!cpu || !cpu->mem) return 0;
    if (!cpu->mem[addr >> 8]) cpu->mem[addr >> 8] = calloc(256, sizeof(char));
    cpu->mem[addr >> 8][addr & 0xFF] = val; 
    return 1;
}

int cpu_ri(CPU* cpu)
{
    if (cpu->psw & (1 << 15)) return 1;
    return 0;
}

int cpu_rd(CPU* cpu)
{
    if (cpu->psw & (1 << 14)) return 1;
    return 0;
}

int cpu_rt(CPU* cpu)
{
    if (cpu->psw & (1 << 13)) return 1;
    return 0;
}

int cpu_rn(CPU* cpu)
{
    if (cpu->psw & (1 << 3)) return 1;
    return 0;
}

int cpu_rc(CPU* cpu)
{
    if (cpu->psw & (1 << 2)) return 1;
    return 0;
}

int cpu_ro(CPU* cpu)
{
    if (cpu->psw & (1 << 1)) return 1;
    return 0;
}

int cpu_rz(CPU* cpu)
{
    if (cpu->psw & (1 << 0)) return 1;
    return 0;
}

void cpu_wi(CPU* cpu, int i)
{
    if (i) cpu->psw |= (1 << 15);
    else cpu->psw &= ~(1 << 15);
}

void cpu_wd(CPU* cpu, int d)
{
    if (d) cpu->psw |= (1 << 14);
    else cpu->psw &= ~(1 << 14);
}

void cpu_wt(CPU* cpu, int t)
{
    if (t) cpu->psw |= (1 << 13);
    else cpu->psw &= ~(1 << 13);
}

void cpu_wn(CPU* cpu, int n)
{
    if (n) cpu->psw |= (1 << 3);
    else cpu->psw &= ~(1 << 3);
}

void cpu_wc(CPU* cpu, int c)
{
    if (c) cpu->psw |= (1 << 2);
    else cpu->psw &= ~(1 << 2);
}

void cpu_wo(CPU* cpu, int o)
{
    if (o) cpu->psw |= (1 << 1);
    else cpu->psw &= ~(1 << 1);
}

void cpu_wz(CPU* cpu, int z)
{
    if (z) cpu->psw |= (1 << 0);
    else cpu->psw &= ~(1 << 0);
}
