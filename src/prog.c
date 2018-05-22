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
    prog->data_head = 0;
    prog->data_tail = 0;
}

void prog_free(Program** prog)
{
    SymbolTableNode* del_sym;
    DataListNode* del_data;
    RelListNode* del_rel;

    while ((*prog)->symbol_table_head)
    {
        del_sym = (*prog)->symbol_table_head;
        (*prog)->symbol_table_head = (*prog)->symbol_table_head->next;
        free(del_sym);
    }
    (*prog)->symbol_table_tail = 0;

    while ((*prog)->data_head)
    {
        del_data = (*prog)->data_head;
        (*prog)->data_head = (*prog)->data_head->next;

        if (del_data->data_buffer) free(del_data->data_buffer);
        del_data->data_buffer = 0;
        del_data->data_size = 0;
        del_data->buffer_size = 0;

        while (del_data->rel_head)
        {
            del_rel = del_data->rel_head;
            del_data->rel_head = del_data->rel_head->next;
            free(del_rel);
        }
        del_data->rel_tail = 0;

        free(del_data);
    }
    (*prog)->data_tail = 0;

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

    if (!prog->symbol_table_head) prog->symbol_table_head = new_node;
    else prog->symbol_table_tail->next = new_node;
    prog->symbol_table_tail = new_node;

    return 0;
}

int prog_make_global(Program* prog, char* label)
{
    SymbolTableNode* node;
    for (node = prog->symbol_table_head; node; node = node->next)
    {
        if (strcmp(label, node->name) == 0)
        {
            node->reach = REACH_GLOBAL;
            return 1;
        }
    }
    
    node = malloc(sizeof(SymbolTableNode));
    node->type = SYM_LABEL;
    strcpy(node->name, label);
    node->sym_id = ++prog->symbol_co;
    node->section_id = 0; // spacial identification for exported and imported?
    node->offset = 0;
    node->reach = REACH_GLOBAL;
    node->next = 0;

    if (!prog->symbol_table_head) prog->symbol_table_head = node;
    else prog->symbol_table_tail->next = node;
    prog->symbol_table_tail = node;

    return 0;
}

void prog_new_seg(Program* prog)
{
    DataListNode* new_node;
    if (!prog) return;

    new_node = malloc(sizeof(DataListNode));
    new_node->data_buffer = calloc(64, sizeof(char));
    new_node->data_size = 0;
    new_node->buffer_size = 64;
    new_node->rel_head = 0;
    new_node->rel_tail = 0;
    new_node->next = 0;

    if (!prog->data_head) prog->data_head = new_node;
    else prog->data_tail->next = new_node;
    prog->data_tail = new_node;
}

void prog_set_seg_len(Program* prog, int len)
{
    SymbolTableNode* section = 0;
    SymbolTableNode* sym;
    for (sym = prog->symbol_table_head; sym; sym = sym->next)
    {
        if (sym->type == SYM_SECTION) section = sym;
    }
    if (!section) return;
    section->offset = len;
}

int prog_add_data(Program* prog, char byte)
{
    if (!prog->data_head) return 0;
    if (prog->data_tail->data_size >= prog->data_tail->buffer_size)
    {
        char* new_buffer;
        prog->data_tail->buffer_size *= 2;
        new_buffer = (char*)realloc(prog->data_tail->data_buffer, prog->data_tail->buffer_size * sizeof(char));
        if (new_buffer == 0) return 0;
        prog->data_tail->data_buffer = new_buffer;
    }

    prog->data_tail->data_buffer[prog->data_tail->data_size++] = byte;
    return 1;
}

int prog_add_rel(Program* prog, int offset, RELOCATION rel, char* sym)
{
    RelListNode* new_rel;
    SymbolTableNode* sym_node;
    if (!prog || !prog->data_head) return 0;

    for (sym_node = prog->symbol_table_head; sym_node; sym_node = sym_node->next)
    {
        if (strcmp(sym_node->name, sym) == 0) break;
    }
    if (!sym_node) return 0;

    new_rel = malloc(sizeof(RelListNode));
    new_rel->offset = offset;
    new_rel->rel = rel;
    new_rel->sym_id = sym_node->sym_id;
    new_rel->next = 0;

    if (!prog->data_tail->rel_head) prog->data_tail->rel_head = new_rel;
    else prog->data_tail->rel_tail->next = new_rel;
    prog->data_tail->rel_tail = new_rel;

    return 1;
}

static int is_hex(char c)
{
    if (c >= '0' && c <= '9') return 1;
    if (c >= 'A' && c <= 'F') return 1;
    return 0;
}

PROG_RET prog_load(Program* prog, char* path)
{
    FILE* file;
    char line[100];

    if (!prog) return PROG_ERT_INVALID_PROGRAM;
    file = fopen(path, "r");
    if (!file) return PROG_RET_INVALID_PATH;

    while(1)
    {
        SymbolTableNode* new_sym;
        int type;
        char name[50];
        int sym_id;
        int section_id;
        int offset;
        int reach;

        if (!fgets(line, sizeof(line), file)) return PROG_ERT_INVALID_PROGRAM;
        if (sscanf(line, "%d %s %d %d %d %d", &type, name, &sym_id, &section_id, &offset, &reach) != 6) break;

        new_sym = malloc(sizeof(SymbolTableNode));
        new_sym->type = (SYM_TYPE)type;
        strcpy(new_sym->name, name);
        new_sym->sym_id = sym_id;
        new_sym->section_id = section_id;
        new_sym->offset = offset;
        new_sym->reach = (SYM_REACH)reach;
        new_sym->next = 0;

        if (!prog->symbol_table_head) prog->symbol_table_head = new_sym;
        else prog->symbol_table_tail->next = new_sym;
        prog->symbol_table_tail = new_sym;
    }
    
    while (line[0])
    {
        int ind = 0;
        if (line[ind++] != '.') return PROG_ERT_INVALID_PROGRAM;
        while (line[ind] && line[ind] != '\n')
        {
            if (line[ind] < 'a' || line[ind] > 'z') return PROG_ERT_INVALID_PROGRAM;
            ind++;
        }

        prog_new_seg(prog);
        while (1)
        {
            RelListNode* new_rel;
            int offset;
            int rel;
            int sym_id;
            char end;

            if (!fgets(line, sizeof(line), file)) return PROG_ERT_INVALID_PROGRAM;
            if (sscanf(line, "%x %d %d%c", &offset, &rel, &sym_id, &end) != 4) break;
            if (end != '\n') break;

            new_rel = malloc(sizeof(RelListNode));
            new_rel->offset = offset;
            new_rel->rel = (RELOCATION)rel;
            new_rel->sym_id = sym_id;
            new_rel->next = 0;

            if (!prog->data_tail->rel_head) prog->data_tail->rel_head = new_rel;
            else prog->data_tail->rel_tail->next = new_rel;
            prog->data_tail->rel_tail = new_rel;
        }
        while (1)
        {
            int valid = 1;
            if (line[0] == '.') break;

            for (ind = 0; line[ind] && line[ind + 1] && line[ind + 2]; ind += 3)
            {
                char byte;
                if (!is_hex(line[ind]) || !is_hex(line[ind+1]) || line[ind+2] != ' ') return PROG_ERT_INVALID_PROGRAM;
                sscanf(line + ind, "%02hhx", &byte);
                prog_add_data(prog, byte);
            }

            if (!fgets(line, sizeof(line), file))
            {
                line[0] = 0;
                break;
            }
        }
    }

    fclose(file);

    return PROG_RET_SUCCESS;
}

PROG_RET prog_store(Program* prog, char* path)
{
    FILE* file;
    SymbolTableNode* sym_node;
    DataListNode* data_node;
    int i;

    if (!prog) return PROG_ERT_INVALID_PROGRAM;
    file = fopen(path, "w");
    if (!file) return PROG_RET_INVALID_PATH;

    for (sym_node = prog->symbol_table_head; sym_node; sym_node = sym_node->next)
    {
        fprintf(file, "%d %s %d %d %d %d\n",
            sym_node->type, sym_node->name, sym_node->sym_id, sym_node->section_id, 
            sym_node->offset, sym_node->reach);
    }

    sym_node = prog->symbol_table_head;
    for (data_node = prog->data_head; data_node; data_node = data_node->next)
    {
        RelListNode* rel_node;

        while (sym_node->type != SYM_SECTION) sym_node = sym_node->next;
        fprintf(file, "%s\n", sym_node->name);
        sym_node = sym_node->next;

        for (rel_node = data_node->rel_head; rel_node; rel_node = rel_node->next)
        {
            fprintf(file, "%08x %d %d\n", rel_node->offset, rel_node->rel, rel_node->sym_id);
        }

        for (i = 0; i < data_node->data_size; i++)
        {
            char c1 = ((data_node->data_buffer[i] >> 4 ) & 0b1111) + '0';
            char c2 = (data_node->data_buffer[i] & 0b1111) + '0';
            if (c1 > '9') c1 = c1 - '9' - 1 + 'A';
            if (c2 > '9') c2 = c2 - '9' - 1 + 'A';
            fprintf(file, "%c%c ", c1, c2);
            if ((i + 1) % 10 == 0) 
            {
                fprintf(file, "\n");
            }
        }
        if ((i + 1) % 10 != 0) fprintf(file, "\n");
    }

    fclose(file);
    return PROG_RET_SUCCESS;
}
