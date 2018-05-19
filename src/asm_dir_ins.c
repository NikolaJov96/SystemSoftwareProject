#include "asm.h"

#include <stdlib.h>

int ins_parse(Instruction* ins, char* line)
{
    if (!ins) return 0;
    return 0;
}

int dir_parse(Directive* dir, char* line)
{
    if (!dir) return 0;
    return 0;
}

void dir_arg_free(Directive* dir)
{
    DirArg* del;
    while (dir->args_head)
    {
        del = dir->args_head;
        dir->args_head = dir->args_head->next;
        free(del);
    }
    dir->args_tail = 0;
}
