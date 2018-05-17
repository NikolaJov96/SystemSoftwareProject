typedef struct
{
    struct SymbolTableNode* next;
} SymbolTableNode;

typedef struct 
{
    SymbolTableNode* symbol_table_head;
    SymbolTableNode* symbol_table_tail;
} Program;

typedef enum { PROG_RET_SUCCESS, PROG_RET_INVALID_PATH, PROG_ERT_INVALID_PROGRAM } PROG_RET;

PROG_RET prog_load(Program** prog, char* path);
PROG_RET prog_store(Program* prog, char* path);
