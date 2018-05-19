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

static int ins_op_parse(char* line, int* start_ind, ADDRESSING* adr, int* reg, int* val)
{
    // ADDR_PSW, ADDR_IMM, ADDR_REGDIR, ADDR_MEMDIR, ADDR_REGINDPOM
    char* str = line + *start_ind;
    {
        int i = 0;
        while (str[i] >= '0' && str[i] <= '9') i++;
        if (!str[i] || str[i] == ' ')
        {
            char num[31];
            if (i > 30) return 0;
            *start_ind += i;
            *adr = ADDR_IMM;
            memcpy(num, str, i);
            num[i + 1] = 0;
            *val = atoi(num);
            return 1;
        }
    }
    {
        int i = 1;
        if (str[0] == '&')
        {

        }
    }
    {
        int i = 0;
        // label
    }
    {
        int i = 1;
        if (str[0] == '*')
        {

        }
    }
    {
        int i = 0;
        int succ = 0;
        if (str[i] == 'r')
        {
            i++;
            if (str[i] >= '0' && str[i] <= '9')
            {
                *reg = str[i] - '0';
                i++;
                if (str[i] == ' ' || str[i] == 0)
                {
                    i++; succ = 1;
                }
                else if (str[i] >= '0' && str[i] <= '9' && (str[i + 1] == ' ' || str[i + 1] == 0))
                {
                    *reg = *reg * 10 + str[i] - '0'; i += 2; succ = 1;
                }
            }
        }
        else if (str[i] == 'p')
        {
            i++;
            if (str[i] == 'c' && (str[i + 1] == ' ' || str[i + 1] == 0))
            {
                *reg = 7; i += 2; succ = 1;
            }
            else if (str[i] == 's' && str[i + 1] == 'w' && (str[i + 2] == ' ' || str[i + 2] == 0))
            {
                *reg = 8; i += 3; succ = 1;
            }
        }
        else if (str[i] == 's')
        {
            i++;
            if (str[i] == 'p' && (str[i + 1] == ' ' || str[i + 1] == 0))
            {
                *reg = 6; i += 2; succ = 1;
            }
        }

        if (succ)
        {
            *start_ind += i;
            *adr = ADDR_REGDIR;
            return 1;
        }
    }
    return 0;
}

int ins_parse(Instruction* ins, char* line)
{
    int ind = 0, ret;
    if (!ins) return 1;

    if (ret = starts_with(line, "add", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_ADD;
    else if (ret = starts_with(line, "sub", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_SUB;
    else if (ret = starts_with(line, "mul", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_MUL;
    else if (ret = starts_with(line, "div", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_DIV;
    else if (ret = starts_with(line, "cmp", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_CMP;
    else if (ret = starts_with(line, "and", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_AND;
    else if (ret = starts_with(line, "or", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_OR;
    else if (ret = starts_with(line, "not", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_NOT;
    else if (ret = starts_with(line, "test", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_TEST;
    else if (ret = starts_with(line, "push", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_PUSH;
    else if (ret = starts_with(line, "pop", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_POP;
    else if (ret = starts_with(line, "call", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_CALL;
    else if (ret = starts_with(line, "iret", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_IRET;
    else if (ret = starts_with(line, "mov", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_MOV;
    else if (ret = starts_with(line, "shl", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_SHL;
    else if (ret = starts_with(line, "shr", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_SHR;
    else if (ret = starts_with(line, "jmp", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_JMP;
    else if (ret = starts_with(line, "ret", &ind, 5, " ", "eq ", "ne ", "gt ", "al ")) ins->ins = INS_RET;
    else return 2;

    switch (ret)
    {
    case 1: ins->cond = COND_AL; break;
    case 2: ins->cond = COND_EQ; break;
    case 3: ins->cond = COND_NE; break;
    case 4: ins->cond = COND_GT; break;
    case 5: ins->cond = COND_AL; break;
    }

    if (!ins_op_parse(line, &ind, &ins->op1_addr, &ins->op1_reg, &ins->op1_val)) return 3;
    if (line[ind] == ' ') ind++;
    if (!ins_op_parse(line, &ind, &ins->op2_addr, &ins->op2_reg, &ins->op2_val)) return 4;
    if (line[ind] != 0) return 5;

    return 0;
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
