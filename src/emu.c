#include "emu.h"

#include <stdio.h>
#include <stdlib.h>

#include "prog.h"

int main(int argc, char** argv)
{
    EmuArgs args;
    Program* prog;
    Program* linked_prog;

    parse_args(argc, argv, &args);

    printf("emulator\n");

    linked_prog = new_program();
    prog = new_program();
    switch(prog_load(prog, "out.txt"))
    {
    case PROG_RET_INVALID_PATH: printf("Invalid path.\n"); exit(1); break;
    case PROG_ERT_INVALID_PROGRAM: printf("Invalid program.\n"); exit(1); break;
    }
    prog_store(prog, "out2.txt");
    prog_free(&prog);

    return 0;
}
