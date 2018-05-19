#include <argp.h>

typedef enum { ARGS_VERB_SILENT, ARGS_VERB_NORMAL, ARGS_VERB_VERBOSE } ARGS_VERB;

typedef enum { COND_EQ, COND_NE, COND_GT, COND_ALL } INS_COND;
typedef enum { INS_ADD, INS_SUB, INS_MUL, INS_DIV, INS_CMP, INS_AND, INS_OR, INS_NOT,
    INS_TEST, INS_PUSH, INS_POP, INS_CALL, INS_IRET, INS_MOV, INS_SHL, INS_SHR } INSTRUCTION;
typedef enum { ADDR_PSW, ADDR_IMM, ADDR_REGDIR, ADDR_MEMDIR, ADDR_REGINDPOM } ADDRESSING;

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
    ADDRESSING op1_addr, op2_addr;
    int op1_reg, op2_reg;
    int op1_val, op2_val;
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
