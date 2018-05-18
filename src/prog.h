
typedef struct
{
    struct SymbolTableNode* next;
} SymbolTableNode;

typedef struct 
{
    SymbolTableNode* symbol_table_head;
    SymbolTableNode* symbol_table_tail;
} Program;

typedef enum { SEC_NONE, SEC_TEXT, SEC_DATA, SEC_RODATA, SEC_BSS, SEC_END } SECTION;
typedef enum { PROG_RET_SUCCESS, PROG_RET_INVALID_PATH, PROG_ERT_INVALID_PROGRAM } PROG_RET;

Program* new_program();
PROG_RET prog_load(Program** prog, char* path);
PROG_RET prog_store(Program* prog, char* path);
