

typedef enum { SEC_NONE, SEC_TEXT, SEC_DATA, SEC_RODATA, SEC_BSS, SEC_END } SECTION;
typedef enum { SYM_SECTION, SYM_LABEL } SYM_TYPE;
typedef enum { REACH_LOCAL, REACH_GLOBAL } SYM_REACH;
typedef enum { PROG_RET_SUCCESS, PROG_RET_INVALID_PATH, PROG_ERT_INVALID_PROGRAM } PROG_RET;

typedef struct SymbolTableNode
{
    SYM_TYPE type;
    char name[50];
    int sym_id;
    int section_id;
    int offset;
    SYM_REACH reach;
    struct SymbolTableNode* next;
} SymbolTableNode;

typedef struct Program
{
    SymbolTableNode* symbol_table_head;
    SymbolTableNode* symbol_table_tail;
    int symbol_co;
} Program;

Program* new_program();
int prog_add_sym(Program* prog, SYM_TYPE type, char* name, int offset);
PROG_RET prog_load(Program** prog, char* path);
PROG_RET prog_store(Program* prog, char* path);
