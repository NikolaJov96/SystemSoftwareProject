#include <stdio.h>

#include "prog.h"

int main(int argc, char** argv)
{
    Program* prog = new_program();
    printf("emulator\n");

    prog_load(prog, "out.txt");
    prog_store(prog, "out2.txt");
    prog_free(&prog);

    return 0;
}
