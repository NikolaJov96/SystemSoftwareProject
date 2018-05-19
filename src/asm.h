#ifndef ASM_H
#define ASM_H

#include <argp.h>

typedef enum { ARGS_VERB_SILENT, ARGS_VERB_NORMAL, ARGS_VERB_VERBOSE } ARGS_VERB;

typedef enum { COND_EQ, COND_NE, COND_GT, COND_AL } INS_COND;
typedef enum 
{ 
    INS_ADD, INS_SUB, INS_MUL, INS_DIV, INS_CMP, INS_AND, INS_OR, INS_NOT, INS_TEST,
    INS_PUSH, INS_POP, INS_CALL, INS_IRET, INS_MOV, INS_SHL, INS_SHR, INS_JMP, INS_RET 
} INSTRUCTION;
typedef enum { ADDR_IMM, ADDR_REGDIR, ADDR_MEMDIR, ADDR_REGINDDISP, ADDR_PCREL } ADDRESSING;

typedef enum { DIR_CHAR, DIR_WORD, DIR_LONG, DIR_ALIGN, DIR_SKIP } DIRECTIVE;

typedef struct AsmArgs
{
    ARGS_VERB verb;
    char input_file_name[256];
    char output_file_name[256];
} AsmArgs;

typedef struct Instruction
{
    INS_COND cond;
    INSTRUCTION ins;
    int num_ops;
    ADDRESSING op1_addr, op2_addr;
    int op1_reg, op2_reg;
    int op1_val, op2_val;
    char op1_label[50], op2_label[50];
} Instruction;

typedef struct DirArg
{
    char str[50];
    struct DirArg* next;
} DirArg;

typedef struct Directive
{
    DIRECTIVE dir;
    DirArg* args_head;
    DirArg* args_tail;
} Directive;

void parse_args(int argc, char** argv, AsmArgs* args);
int check_reserved(char* word);

int ins_parse(Instruction* ins, char* line);
int ins_valid_addr(INSTRUCTION ins, ADDRESSING addr, int op_ind);
int ins_len(INSTRUCTION ins);
int dir_parse(Directive* dir, char* line);
void dir_arg_free(Directive* dir);

#endif  // ASM_H
