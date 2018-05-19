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

int ins_parse(Instruction* ins, char* line)
{
    int ind = 0, ret;
    /*ins->cond
    ins->ins
    ins->op1_addr
    ins->op2_addr
    ins->op1_reg
    ins->op2_reg
    ins->op1_val
    ins->op2_val*/
    if (!ins) return 0;

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
    else return 0;

    switch (ret)
    {
    case 1: ins->cond = COND_AL; break;
    case 2: ins->cond = COND_EQ; break;
    case 3: ins->cond = COND_NE; break;
    case 4: ins->cond = COND_GT; break;
    case 5: ins->cond = COND_AL; break;
    }

    return 1;
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
