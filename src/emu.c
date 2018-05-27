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
        switch(prog_load(prog, LOAD_FILE, input_node->file))
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

    if (args.verb == ARGS_VERB_VERBOSE) printf("Linking combined file.\n");

    if (!prog_relocate(linked_prog))
    {
        sprintf(err_line, "Error resolving local relocations in the linked program.\n");
        exit_prog(args.verb, 0);
    }

    prog = new_program();
    prog_link(prog, linked_prog);
    prog_free(&linked_prog);

    if (!prog_test_addr(prog))
    {
        sprintf(err_line, "Sections overlapping in the linked program.\n");
        exit_prog(args.verb, 0);
    }

    if (args.do_link)
    {
        if (args.verb == ARGS_VERB_VERBOSE) printf("Saving object file.\n");
        if (prog_store(prog, args.link_target) != PROG_RET_SUCCESS)
        {
            printf("Colud not save linked object file\n"); 
            exit(1);
        }
    }

    if (args.do_build)
    {
        FILE* file;
        char line[250];
        char** prog_arr = calloc(64, sizeof(char*));
        int arr_len = 0, buff_len = 64, i;
        
        if (args.verb == ARGS_VERB_VERBOSE) printf("Building executable.\n");
        
        if (prog_store(prog, "src/temp_file.c") != PROG_RET_SUCCESS)
        {
            printf("Colud not generate runnable\n"); 
            exit(1);
        }
        file = fopen("src/temp_file.c", "r");
        if (!file)
        {
            printf("Colud not generate runnable\n");
            strcpy(line, "rm ");
            strcat(line, "src/temp_file.c");
            system(line);
            exit(1);
        }
        while (fgets(line, sizeof(line), file))
        {
            int line_len = strlen(line);
            if (arr_len >= buff_len)
            {
                buff_len *= 2;
                prog_arr = realloc(prog_arr, buff_len * sizeof(char*));
            }
            prog_arr[arr_len] = calloc(line_len + 1, sizeof(char));
            strcpy(prog_arr[arr_len], line);
            arr_len++;
        }
        if (arr_len >= buff_len)
        {
            buff_len *= 2;
            prog_arr = realloc(prog_arr, buff_len * sizeof(char*));
        }
        prog_arr[arr_len++] = calloc(1, sizeof(char));
        fclose(file);
        
        file = fopen("src/temp_file.c", "w");
        fprintf(file, "char* prgo_cont[250] = {\n");
        
        for (i = 0; i < arr_len - 1; i++)
        {
            int line_len = strlen(prog_arr[i]);
            if (line_len > 1)
            {
                prog_arr[i][line_len - 1] = 0;
            }
            fprintf(file, "    \"%s\\n\",\n", prog_arr[i]);
        }
        fprintf(file, "    \"%s\"\n", prog_arr[arr_len - 1]);
        fprintf(file, "};\n");
        fprintf(file, "#include \"prog.h\"\n");
        fprintf(file, "#include \"emu.h\"\n");
        fprintf(file, "int main()\n");
        fprintf(file, "{\n");
        fprintf(file, "    Program* prog = new_program();\n");
        fprintf(file, "    prog_load(prog, LOAD_STR_ARR, prgo_cont);\n");
        fprintf(file, "    emu_run(prog, 1);\n");
        fprintf(file, "    prog_free(&prog);\n");
        fprintf(file, "}\n");
        fclose(file);
        for (i = 0; i < arr_len; i++) free(prog_arr[i]);
        free(prog_arr);
        
        strcpy(line, "gcc -o ");
        strcat(line, args.build_target);
        strcat(line, " src/temp_file.c src/prog.c src/emu_run.c");
        system(line);

        strcpy(line, "rm ");
        strcat(line, "src/temp_file.c");
        system(line);
    }
    
    if (args.do_run)
    {
        if (args.verb == ARGS_VERB_VERBOSE) printf("Running emulation.\n");
        emu_run(prog, args.verb == ARGS_VERB_VERBOSE);
    }

    if (args.verb == ARGS_VERB_VERBOSE) printf("\nDone.\n");

    prog_free(&prog);
    return 0;
}
