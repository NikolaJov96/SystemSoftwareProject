#ifndef PROG_H
#define PROG_H

typedef enum { SEC_NONE, SEC_TEXT, SEC_DATA, SEC_RODATA, SEC_BSS, SEC_END } SECTION;
typedef enum { SYM_SECTION, SYM_LABEL } SYM_TYPE;
typedef enum { REACH_LOCAL, REACH_GLOBAL } SYM_REACH;
typedef enum { REL_16, REL_PC16 } RELOCATION;
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

typedef struct RelListNode
{
    int offset;
    RELOCATION rel;
    int sym_id;
    struct RelListNode* next;
} RelListNode;

typedef struct DataListNode
{
    char* data_buffer;
    int data_size;
    int buffer_size;
    RelListNode* rel_head;
    RelListNode* rel_tail;
    struct DataListNode* next;
} DataListNode;

typedef struct Program
{
    SymbolTableNode* symbol_table_head;
    SymbolTableNode* symbol_table_tail;
    int symbol_co;
    DataListNode* data_head;
    DataListNode* data_tail;
    int section_co;
} Program;

Program* new_program();
void prog_free(Program** prog);

int prog_add_sym(Program* prog, SYM_TYPE type, char* name, int offset);
int prog_make_global(Program* prog, char* label);

void prog_new_seg(Program* prog);
void prog_set_seg_len(Program* prog, int len);
int prog_add_data(Program* prog, char byte);
int prog_add_rel(Program* prog, int offset, RELOCATION rel, char* sym);

int prog_relocate(Program* prog);
int prog_link(Program* dst, Program* src);

PROG_RET prog_load(Program* prog, char* path);
PROG_RET prog_store(Program* prog, char* path);

#endif  // PROG_H
