#include "prog.h"

#include <stdlib.h>

Program* new_program()
{
    Program* prog = malloc(sizeof(Program));
    prog->symbol_table_head = 0;
    prog->symbol_table_tail = 0;
}

PROG_RET prog_load(Program** prog, char* path)
{
    prog = 0;
    return PROG_RET_SUCCESS;
}

PROG_RET prog_store(Program* prog, char* path)
{
    return PROG_RET_SUCCESS;
}
