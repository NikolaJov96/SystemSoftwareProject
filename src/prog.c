#include "prog.h"

#include <stdio.h>
#include <stdlib.h>

Program* new_program()
{
    Program* prog = malloc(sizeof(Program));
    prog->symbol_table_head = 0;
    prog->symbol_table_tail = 0;
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

    if (!prog) return PROG_ERT_INVALID_PROGRAM;
    file = fopen(path, "w");
    if (!file) return PROG_RET_INVALID_PATH;

    fprintf(file, "assembler\n");

    fclose(file);
    return PROG_RET_SUCCESS;
}
