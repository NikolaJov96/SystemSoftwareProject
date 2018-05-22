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

int check_reserved(char* word)
{
    int ind = 0;
    if (starts_with(word, "add",  &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "sub",  &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "mul",  &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "div",  &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "cmp",  &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "and",  &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "or",   &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "not",  &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "test", &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "push", &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "pop",  &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "call", &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "iret", &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "mov",  &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "shl",  &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "shr",  &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "jmp",  &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (starts_with(word, "ret",  &ind, 5, "", "eq", "ne", "gt", "al")) return 1;
    if (strcmp(word, ".char") == 0) return 1;
    if (strcmp(word, ".word") == 0) return 1;
    if (strcmp(word, ".long") == 0) return 1;
    if (strcmp(word, ".align") == 0) return 1;
    if (strcmp(word, ".skip") == 0) return 1;
    if (strcmp(word, ".global") == 0) return 1;
    if (strcmp(word, "sp") == 0) return 1;
    if (strcmp(word, "pc") == 0) return 1;
    if (strcmp(word, "psw") == 0) return 1;
    return 0;
}

static InsOp* ins_op_parse(char* line, int* start_ind)
{
    char* str = line + *start_ind;
    int reg = 0;

    {
        int i = 0;
        int start = 0;
        if (str[i] == '*')
        {
            i++;
            start = 1;
        }
        else if (str[i] == '-') i++;
        if (str[i] >= '0' && str[i] <= '9')
        {
            while (str[i] >= '0' && str[i] <= '9') i++;
            if (str[i] == ' ' || str[i] == 0)
            {
                char num[31];
                InsOp* new_op;
                i++;
                if (i > 30) return 0;

                *start_ind += i - 1;
                new_op = calloc(1, sizeof(InsOp));
                new_op->addr = ( start == 0 ? ADDR_IMM : ADDR_MEMDIR );
                memcpy(num, str + start, i - start);
                num[i - start] = 0;
                new_op->val = atoi(num);
                return new_op;
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
                char label[50];
                InsOp* new_op;
                memcpy(label, str + start, i - start);
                label[i - start] = 0;
                if (check_reserved(label)) return 0;
                
                *start_ind += i;
                new_op = calloc(1, sizeof(InsOp));
                switch (status)
                {
                case 0: new_op->addr = ADDR_MEMDIR; break;
                case 1: new_op->addr = ADDR_IMM; break;
                case 2: new_op->addr = ADDR_PCREL; break;
                }
                memcpy(new_op->label, str + start, i - start);
                new_op->label[i - start] = 0;
                return new_op;
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
                reg = str[i] - '0';
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
                reg = 7; 
                i++;
                if (str[i] == ' ' || str[i] == 0) { i++; status = 1; }
                else if (str[i] == '[')  { i++; status = 2; }
            }
            else if (str[i] == 's' && str[i + 1] == 'w')
            {
                reg = 8;
                i += 2;
                if (str[i] == ' ' || str[i] == 0) 
                {
                    InsOp* new_op = calloc(1, sizeof(InsOp));
                    i++; 
                    *start_ind += i - 1;
                    new_op->addr = ADDR_PSW;
                    return new_op;
                }
            }
        }
        else if (str[i] == 's')
        {
            i++;
            if (str[i] == 'p')
            {
                reg = 6;
                i++;
                if (str[i] == ' ' || str[i] == 0) { i++; status = 1; }
                else if (str[i] == '[')  { i++; status = 2; }
            }
        }

        if (status == 1)
        {
            InsOp* new_op = calloc(1, sizeof(InsOp));
            *start_ind += i - 1;
            new_op->addr = ADDR_REGDIR;
            new_op->reg = reg;
            return new_op;
        }
        else if (status == 2)
        {
            int disp_start = i;
            if (str[i] == '-' && str[i + 1] >= '0' && str[i + 1] <= '9') i++;
            if (str[i] >= '0' && str[i] <= '9')
            {
                while (str[i] >= '0' && str[i] <= '9') i++;
                if (str[i] == ']' && i < 30 && (str[i + 1] == ' ' || str[i + 1] == 0))
                {
                    char num[31];
                    InsOp* new_op = calloc(1, sizeof(InsOp));

                    *start_ind += i + 1;
                    new_op->addr = ADDR_REGINDDISP;
                    memcpy(num, str + disp_start, i - disp_start);
                    num[i - disp_start] = 0;
                    new_op->val = atoi(num);
                    return new_op;
                }
            }
            else if (str[i] >= 'a' && str[i] <= 'z')
            {
                while (str[i] >= 'a' && str[i] <= 'z') i++;
                if (str[i] == ']' && i < 50 && (str[i + 1] == ' ' || str[i + 1] == 0))
                {
                    char label[50];
                    InsOp* new_op;
                    memcpy(label, str + disp_start, i - disp_start);
                    label[i - disp_start] = 0;
                    if (check_reserved(label)) return 0;

                    new_op = calloc(1, sizeof(InsOp));
                    *start_ind += i + 1;
                    new_op->addr = ADDR_REGINDDISP;
                    memcpy(new_op->label, str + disp_start, i - disp_start);
                    new_op->label[i - disp_start] = 0;
                    return new_op;
                }
            }
        }
    }
    return 0;
}

static void ins_add_op(Instruction* ins, InsOp* arg)
{
    arg->next = 0;
    if (!ins->ops_head)
    {
        ins->ops_head = arg;
        ins->ops_tail = arg;
    }
    else
    {
        ins->ops_tail->next = arg;
        ins->ops_tail = arg;
    }
    ins->num_ops++;
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
    else if (ret = starts_with(line, "ret",  &ind, 5, "al",  "eq",  "ne",  "gt",  "" )) ins->ins = INS_RET;
    else return 2;
    if (line[ind] == ' ') ind++;

    switch (ret)
    {
    case 1: ins->cond = COND_AL; break;
    case 2: ins->cond = COND_EQ; break;
    case 3: ins->cond = COND_NE; break;
    case 4: ins->cond = COND_GT; break;
    case 5: ins->cond = COND_AL; break;
    }

    ins->ops_head = 0;
    ins->ops_tail = 0;
    ins->num_ops = 0;
    while (line[ind] != 0) 
    {
        ins->num_ops++;
        InsOp* new_op = ins_op_parse(line, &ind);
        if (!new_op) return 3;
        ins_add_op(ins, new_op);
        if (line[ind] == ' ') ind++;
    }

    return 0;
}

int ins_valid_addr(INSTRUCTION ins, ADDRESSING addr, int op_ind)
{
    switch (ins)
    {
    case INS_ADD: case INS_SUB: case INS_MUL: case INS_DIV: case INS_AND: 
    case INS_OR: case INS_NOT: case INS_SHL: case INS_SHR: case INS_MOV: // mov explanation
        if (op_ind > 1) return 2;
        if (op_ind > 0 || addr != ADDR_IMM) return 0;
        return 1;
        break;
    case INS_CMP: 
        if (op_ind > 1) return 2;
        return 0; 
        break;
    case INS_TEST:  // what is the result of test?
        if (op_ind > 1) return 2;
        return 0;
        break;
    case INS_PUSH: case INS_CALL: case INS_JMP:
        if (op_ind > 0) return 2; 
        return 0;
        break;
    case INS_POP: case INS_IRET:
        if (op_ind > 0) return 2;
        if (addr == ADDR_IMM) return 1;
        return 0;
        break;
    case INS_RET: return 2;
    }
}

int ins_len(ADDRESSING addr)
{
    switch (addr)
    {
    case ADDR_PSW: case ADDR_REGDIR: return 2; break;
    case ADDR_IMM: case ADDR_MEMDIR: case ADDR_REGINDDISP: case ADDR_PCREL: return 4; break;
    }
    return 0;
}

void ins_op_free(Instruction* ins)
{
    InsOp* del;
    while (ins->ops_head)
    {
        del = ins->ops_head;
        ins->ops_head = ins->ops_head->next;
        free(del);
    }
    ins->ops_tail = 0;
    ins->num_ops = 0;
}

static void dir_add_arg(Directive* dir, DirArg* arg)
{
    arg->next = 0;
    if (!dir->args_head)
    {
        dir->args_head = arg;
        dir->args_tail = arg;
    }
    else
    {
        dir->args_tail->next = arg;
        dir->args_tail = arg;
    }
    dir->num_args++;
}

int dir_parse(Directive* dir, char* line)
{
    int ind = 0;
    if (!dir) return 1;

    if      (starts_with(line, ".char ",   &ind, 0)) dir->dir = DIR_CHAR;
    else if (starts_with(line, ".word ",   &ind, 0)) dir->dir = DIR_WORD;
    else if (starts_with(line, ".long ",   &ind, 0)) dir->dir = DIR_LONG;
    else if (starts_with(line, ".align ",  &ind, 0)) dir->dir = DIR_ALIGN;
    else if (starts_with(line, ".skip ",   &ind, 0)) dir->dir = DIR_SKIP;
    else if (starts_with(line, ".global ", &ind, 0)) dir->dir = DIR_GLOBAL;
    else return 2;

    dir->args_head = 0;
    dir->args_tail = 0;
    dir->num_args = 0;
    while (line[ind] != 0)
    {
        int start = ind;

        if (line[ind] == '-') ind++;
        if (line[ind] >= '0' && line[ind] <= '9')
        {
            while (line[ind] >= '0' && line[ind] <= '9') ind++;
            if ((line[ind] == ',' && line[ind + 1] == ' ') || line[ind] == 0)
            {
                char num[31];
                DirArg* new_arg;

                if (ind - start > 30) { dir_arg_free(dir); return 4; }
                new_arg = malloc(sizeof(DirArg));
                new_arg->label[0] = 0;
                memcpy(num, line + start, ind - start);
                num[ind - start] = 0;
                new_arg->val = atoi(num);
                dir_add_arg(dir, new_arg);
            }
            else { dir_arg_free(dir); return 6; }
        }
        else if (line[ind] >= 'a' && line[ind] <= 'z')
        {
            while (line[ind] >= 'a' && line[ind] <= 'z') ind++;
            if (ind - start < 50 && (line[ind] == ',' && line[ind + 1] == ' ') || line[ind] == 0)
            {
                char label[50];
                DirArg* new_arg;

                memcpy(label, line + start, ind - start);
                label[ind - start] = 0;
                if (check_reserved(label)) { dir_arg_free(dir); return 5; }

                new_arg = malloc(sizeof(DirArg));
                memcpy(new_arg->label, label, ind - start);
                new_arg->label[ind-start] = 0;
                new_arg->val = 0;
                dir_add_arg(dir, new_arg);
            }
            else { dir_arg_free(dir); return 6; }
        }
        else { dir_arg_free(dir); return 3; }

        if (line[ind] == 0) break;
        if (line[ind] != ',' || line[ind + 1] != ' ') { dir_arg_free(dir); return 7; }
        ind += 2;
    }

    return 0;
}

int dir_len(Directive* dir, int curr_offset)
{
    int len = 0;
    if (!dir) return 0;
    switch (dir->dir)
    {
    case DIR_CHAR: len = 1; break;
    case DIR_WORD: len = 2; break;
    case DIR_LONG: len = 4; break;
    case DIR_ALIGN:
        if (dir->num_args != 1 || dir->args_head->label[0] != 0 || dir->args_head->val < 0) return -1;
        if (curr_offset % dir->args_head->val == 0) return 0;
        return (curr_offset / dir->args_head->val + 1) * dir->args_head->val - curr_offset;
        break;
    case DIR_SKIP:
        if (dir->num_args != 1 || dir->args_head->label[0] != 0 || dir->args_head->val < 0) return -1;
        return dir->args_head->val;
        break;
    case DIR_GLOBAL: return 0; break;
    }
    return len * dir->num_args;
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
    dir->num_args = 0;
}
