#include "emu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prog.h"

static char err_line[256];
static void exit_prog(ARGS_VERB verb, FILE *close_file)
{
    if (verb != ARGS_VERB_SILENT)  printf("%s\n", err_line);
    if (close_file) fclose(close_file);
    exit(1);
}

int main(int argc, char** argv)
{
    EmuArgs args;
    FILE* file;
    Program* prog;
    Program* linked_prog;
    InputListNode* input_node;
    int file_ind;

    parse_args(argc, argv, &args);

    for (input_node = args.in_head; input_node; input_node = input_node->next)
    {
        file = fopen(input_node->file, "r");
        if (!file)
        {
            sprintf(err_line, "Error opening input file %s : %s", input_node->file, strerror(errno));
            exit_prog(args.verb, file);
        }
        fclose(file);
    }
    if (args.do_link)
    {
        file = fopen(args.link_target, "w");
        if (!file)
        {
            sprintf(err_line, "Error opening link target file %s : %s\n", args.link_target, strerror(errno));
            exit_prog(args.verb, file);
        }
        fclose(file);
    }
    if (args.do_build)
    {
        file = fopen(args.build_target, "w");
        if (!file)
        {
            sprintf(err_line, "Error opening build trget file %s : %s\n", args.build_target, strerror(errno));
            exit_prog(args.verb, file);
        }
        fclose(file);
    }

    linked_prog = new_program();
    for (input_node = args.in_head, file_ind = 0; input_node; input_node = input_node->next, file_ind++)
    {
        prog = new_program();
        switch(prog_load(prog, input_node->file))
        {
        case PROG_RET_INVALID_PATH: 
            printf("Cannot open input file: %s.\n", input_node->file); exit(1); break;
        case PROG_ERT_INVALID_PROGRAM: 
            printf("Input file invalid content: %s.\n", input_node->file); exit(1); break;
        }

        if (args.verb == ARGS_VERB_VERBOSE)
        {
            printf("  Linking file: %s\n", input_node->file);
        }

        if (!prog_relocate(prog))
        {
            sprintf(err_line, "Error resolving local relocations in %s\n", input_node->file);
            exit_prog(args.verb, file);
        }

        if (!prog_link(linked_prog, prog))
        {
            sprintf(err_line, "Error linking %s to the rest.\n", input_node->file);
            exit_prog(args.verb, file);
        }

        prog_free(&prog);
    }

    if (args.verb == ARGS_VERB_VERBOSE) printf("  Linking combined file.\n");

    if (!prog_relocate(linked_prog))
    {
        sprintf(err_line, "Error resolving local relocations in the linked program.\n");
        exit_prog(args.verb, 0);
    }

    prog = new_program();
    prog_link(prog, linked_prog);

    if (!prog_test_addr(prog))
    {
        sprintf(err_line, "Sections overlapping in the linked program.\n");
        exit_prog(args.verb, 0);
    }

    if (args.do_link)
    {
        if (args.verb == ARGS_VERB_VERBOSE) printf("  Saving object file.\n");
        if (prog_store(prog, args.link_target) != PROG_RET_SUCCESS)
        {
            printf("Colud not save linked object file\n"); 
            exit(1);
        }
    }

    if (args.do_build)
    {
        // generate executable
    }

    if (args.do_run)
    {
        if (args.verb == ARGS_VERB_VERBOSE) printf("  Running emulation.\n");
        emu_run(prog);
    }

    if (args.verb == ARGS_VERB_VERBOSE) printf("\n  Done.\n");

    prog_free(&linked_prog);
    return 0;
}
