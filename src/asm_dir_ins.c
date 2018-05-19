#include "asm.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int starts_with(char* str1, char* str2, int* start_ind, int suffixes, ...)
{
    int i = 0, j, str2_len;
    va_list vl;
    if (strlen(str1) <= *start_ind) return 0;
    while (str2[i])
    {
        if (str1[*start_ind + i] != str2[i]) return 0;
        i++;
    }
    if (suffixes == 0)
    {
        *start_ind += i;
        return 1;
    }

    str2_len = i;

    va_start(vl, suffixes);
    for (j = 0; j < suffixes; j++)
    {
        i = 0;
        char* suff = va_arg(vl, char*);
        while (suff[i])
        {
            if (str1[*start_ind + str2_len + i] != suff[i]) break;
            i++;
        }
        if (!suff[i])
        {
            va_end(vl);
            *start_ind += str2_len + i;
            return j + 1;
        }
    }

    va_end(vl);
    return 0;
}

static int ins_op_parse(char* line, int* start_ind, ADDRESSING* adr, int* reg, int* val, char* label)
{
    char* str = line + *start_ind;
    *reg = -1;
    *val = 0;
    label[0] = 0;

    {
        int i = 0;
        int start = 0;
        if (str[i] == '*')
        {
            i++;
            start = 1;
        }
        if (str[i] >= '0' && str[i] <= '9')
        {
            while (str[i] >= '0' && str[i] <= '9') i++;
            if (str[i] == ' ' || str[i] == 0)
            {
                i++;
                char num[31];
                if (i > 30) return 0;
                *start_ind += i - 1;
                *adr = ( start == 0 ? ADDR_IMM : ADDR_MEMDIR );
                memcpy(num, str + start, i - start);
                num[i - start] = 0;
                *val = atoi(num);
                return 1;
            }
        }
    }
    {
        int i = 0, start = 0, status = 0;

        if (str[i] == '&') { i++; start = 1; status = 1; }
        else if (str[i] == '$') { i++; start = 1; status = 2; }
        if (str[i] >= 'a' && str[i] <= 'z')
        {
            while (str[i] >= 'a' && str[i] <= 'z') i++;
            if (i < 50 && (str[i] == ' ' || str[i] == 0))
            {
                *start_ind += i;
                switch (status)
                {
                case 0: *adr = ADDR_MEMDIR; break;
                case 1: *adr = ADDR_IMM; break;
                case 2: *adr = ADDR_PCREL; break;
                }
                memcpy(label, str + start, i - start);
                label[i - start] = 0;
                if (!check_reserved(label)) return 1;
            }
        }
    }
    {
        int i = 0;
        int status = 0;
        if (str[i] == 'r')
        {
            i++;
            if (str[i] >= '0' && str[i] <= '7')
            {
                *reg = str[i] - '0';
                i++;
                if (str[i] == ' ' || str[i] == 0) { i++; status = 1; }
                else if (str[i] == '[') { i++; status = 2; }
            }
        }
        else if (str[i] == 'p')
        {
            i++;
            if (str[i] == 'c')
            {
                *reg = 7; 
                i++;
                if (str[i] == ' ' || str[i] == 0) { i++; status = 1; }
                else if (str[i] == '[')  { i++; status = 2; }
            }
            else if (str[i] == 's' && str[i + 1] == 'w')
            {
                *reg = 8;
                i += 2;
                if (str[i] == ' ' || str[i] == 0) 
                { 
                    i++; 
                    *start_ind += i - 1;
                    *adr = ADDR_PSW;
                    return 1;
                }
            }
        }
        else if (str[i] == 's')
        {
            i++;
            if (str[i] == 'p')
            {
                *reg = 6;
                i++;
                if (str[i] == ' ' || str[i] == 0) { i++; status = 1; }
                else if (str[i] == '[')  { i++; status = 2; }
            }
        }

        if (status == 1)
        {
            *start_ind += i - 1;
            *adr = ADDR_REGDIR;
            return 1;
        }
        else if (status == 2)
        {
            int disp_start = i;
            if (str[i] >= '0' && str[i] <= '9')
            {
                while (str[i] >= '0' && str[i] <= '9') i++;
                if (str[i] == ']' && i < 30 && (str[i + 1] == ' ' || str[i + 1] == 0))
                {
                    i += 2;
                    char num[31];
                    *start_ind += i - 1;
                    *adr = ADDR_REGINDDISP;
                    memcpy(num, str + disp_start, i);
                    num[i] = 0;
                    *val = atoi(num);
                    return 1;
                }
            }
            else if (str[i] >= 'a' && str[i] <= 'z')
            {
                while (str[i] >= 'a' && str[i] <= 'z') i++;
                if (str[i] == ']' && i < 50 && (str[i + 1] == ' ' || str[i + 1] == 0))
                {
                    i += 2;
                    *start_ind += i - 1;
                    *adr = ADDR_REGINDDISP;
                    memcpy(label, str + disp_start, i - 1);
                    label[i] = 0;
                    if (!check_reserved(label)) return 1;
                }
            }
        }
    }
    return 0;
}

int ins_parse(Instruction* ins, char* line)
{
    int ind = 0, ret;
    if (!ins) return 1;

    if      (ret = starts_with(line, "add",  &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_ADD;
    else if (ret = starts_with(line, "sub",  &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_SUB;
    else if (ret = starts_with(line, "mul",  &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_MUL;
    else if (ret = starts_with(line, "div",  &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_DIV;
    else if (ret = starts_with(line, "cmp",  &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_CMP;
    else if (ret = starts_with(line, "and",  &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_AND;
    else if (ret = starts_with(line, "or",   &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_OR;
    else if (ret = starts_with(line, "not",  &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_NOT;
    else if (ret = starts_with(line, "test", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_TEST;
    else if (ret = starts_with(line, "push", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_PUSH;
    else if (ret = starts_with(line, "pop",  &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_POP;
    else if (ret = starts_with(line, "call", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_CALL;
    else if (ret = starts_with(line, "iret", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_IRET;
    else if (ret = starts_with(line, "mov",  &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_MOV;
    else if (ret = starts_with(line, "shl",  &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_SHL;
    else if (ret = starts_with(line, "shr",  &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_SHR;
    else if (ret = starts_with(line, "jmp",  &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_JMP;
    else if (ret = starts_with(line, "ret",  &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_RET;
    else return 2;

    switch (ret)
    {
    case 1: ins->cond = COND_AL; break;
    case 2: ins->cond = COND_EQ; break;
    case 3: ins->cond = COND_NE; break;
    case 4: ins->cond = COND_GT; break;
    case 5: ins->cond = COND_AL; break;
    }

    ins->num_ops = 0;
    if (!ins_op_parse(line, &ind, &ins->op1_addr, &ins->op1_reg, &ins->op1_val, ins->op1_label)) return 3;
    ins->num_ops = 1;
    if (line[ind] == 0) return 0;
    if (line[ind] == ' ') ind++;
    if (!ins_op_parse(line, &ind, &ins->op2_addr, &ins->op2_reg, &ins->op2_val, ins->op2_label)) return 4;
    ins->num_ops = 2;
    if (line[ind] != 0) return 5;

    return 0;
}

int ins_valid_addr(INSTRUCTION ins, ADDRESSING addr, int op_ind)
{
    switch (ins)
    {
        /*case INS_ADD, INS_SUB, INS_MUL, INS_DIV, INS_CMP, INS_AND, INS_OR, INS_NOT, INS_TEST,
    INS_PUSH, INS_POP, INS_CALL, INS_IRET, INS_MOV, INS_SHL, INS_SHR, INS_JMP, INS_RET */
    }
    return 0;
}

int ins_len(ADDRESSING addr)
{
    switch (addr)
    {
    case ADDR_PSW: case ADDR_REGDIR: return 2; break;
    case ADDR_IMM: case ADDR_MEMDIR: case ADDR_REGINDDISP: case ADDR_PCREL: return 4; break;
    }
}

int dir_parse(Directive* dir, char* line)
{
    if (!dir) return 0;
    return 0;
}

void dir_arg_free(Directive* dir)
{
    DirArg* del;
    while (dir->args_head)
    {
        del = dir->args_head;
        dir->args_head = dir->args_head->next;
        free(del);
    }
    dir->args_tail = 0;
}
