#include "prog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Program* new_program()
{
    Program* prog = malloc(sizeof(Program));
    prog->symbol_table_head = 0;
    prog->symbol_table_tail = 0;
    prog->symbol_co = 0;
}

void prog_free(Program** prog)
{
    SymbolTableNode* del;
    while ((*prog)->symbol_table_head)
    {
        del = (*prog)->symbol_table_head;
        (*prog)->symbol_table_head = (*prog)->symbol_table_head->next;
        free(del);
    }
    (*prog)->symbol_table_tail = 0;
    free(*prog);
    *prog = 0;
}

int prog_add_sym(Program* prog, SYM_TYPE type, char* name, int offset)
{
    SymbolTableNode* new_node;
    if (!prog || !name || strlen(name) >= 50) return 0;
    new_node = malloc(sizeof(SymbolTableNode));
    new_node->type = type;
    strcpy(new_node->name, name);
    new_node->sym_id = ++prog->symbol_co;
    new_node->section_id = ( type == SYM_SECTION ? new_node->sym_id : prog->symbol_table_tail->sym_id );
    new_node->offset = offset;
    new_node->reach = REACH_LOCAL;
    new_node->next = 0;
    if (!prog->symbol_table_head)
    {
        prog->symbol_table_head = new_node;
        prog->symbol_table_tail = new_node;
    }
    else
    {
        prog->symbol_table_tail->next = new_node;
        prog->symbol_table_tail = new_node;
    }
}

PROG_RET prog_load(Program** prog, char* path)
{
    FILE* file;

    prog = 0;
    file = fopen(path, "r");
    if (!file) return PROG_RET_INVALID_PATH;

    // fscanf()

    fclose(file);

    return PROG_RET_SUCCESS;
}

PROG_RET prog_store(Program* prog, char* path)
{
    FILE* file;
    SymbolTableNode* node;

    if (!prog) return PROG_ERT_INVALID_PROGRAM;
    file = fopen(path, "w");
    if (!file) return PROG_RET_INVALID_PATH;

    for (node = prog->symbol_table_head; node; node = node->next)
    {
        fprintf(file, "%d %s %d %d %d %d\n",
            node->type, node->name, node->sym_id, node->section_id, node->offset, node->reach);
    }

    fclose(file);
    return PROG_RET_SUCCESS;
}
