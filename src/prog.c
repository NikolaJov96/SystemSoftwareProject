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
    prog->data_buffer = calloc(64, sizeof(char));
    prog->data_size = 0;
    prog->buffer_size = 64;
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

    if ((*prog)->data_buffer) free((*prog)->data_buffer);
    (*prog)->data_buffer = 0;
    (*prog)->data_size = 0;
    (*prog)->buffer_size = 0;

    free(*prog);
    *prog = 0;
}

int prog_add_sym(Program* prog, SYM_TYPE type, char* name, int offset)
{
    SymbolTableNode* new_node;
    SymbolTableNode* temp_node = prog->symbol_table_head;
    if (!prog || !name || strlen(name) >= 50) return 1;
    while (temp_node)
    {
        if (strcmp(temp_node->name, name) == 0) return 2;
        temp_node = temp_node->next;
    }

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
    return 0;
}

int prog_add_data(Program* prog, char byte)
{
    if (prog->data_size >= prog->buffer_size)
    {
        char* new_buffer;
        prog->buffer_size *= 2;
        new_buffer = (char*)realloc(prog->data_buffer, prog->buffer_size * sizeof(char));
        if (new_buffer == 0) return 0;
        prog->data_buffer = new_buffer;
    }

    prog->data_buffer[prog->data_size++] = byte;
    return 1;
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
    int i;

    if (!prog) return PROG_ERT_INVALID_PROGRAM;
    file = fopen(path, "w");
    if (!file) return PROG_RET_INVALID_PATH;

    for (node = prog->symbol_table_head; node; node = node->next)
    {
        fprintf(file, "%d %s %d %d %d %d\n",
            node->type, node->name, node->sym_id, node->section_id, node->offset, node->reach);
    }

    for (i = 0; i < prog->data_size; i++)
    {
        char c1 = ((prog->data_buffer[i] >> 4 ) & 0b1111) + '0';
        char c2 = (prog->data_buffer[i] & 0b1111) + '0';
        if (c1 > '9') c1 = c1 - '9' - 1 + 'A';
        if (c2 > '9') c2 = c2 - '9' - 1 + 'A';
        fprintf(file, "%c%c ", c1, c2);
        if ((i + 1) % 10 == 0) 
        {
            fprintf(file, "\n");
        }
    }

    fclose(file);
    return PROG_RET_SUCCESS;
}
