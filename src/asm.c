#include "asm.h"

#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>

#include "prog.h"

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    AsmArgs* args = state->input;
    FILE* file;
    // printf("arg: '%d' '%s'\n", key, arg);
    switch (key) {
    case ARGP_KEY_INIT:
        args->verb = ARGS_VERB_NORMAL;
        args->input_file_name[0] = 0;
        args->output_file_name[0] = 0;
        break;
    case 'v':
        if (args->verb != ARGS_VERB_NORMAL) return ARGP_ERR_UNKNOWN;
        else args->verb = ARGS_VERB_VERBOSE;
        break;
    case 's': 
        if (args->verb != ARGS_VERB_NORMAL) return ARGP_ERR_UNKNOWN;
        else args->verb = ARGS_VERB_SILENT;
        break;
    case ARGP_KEY_ARG:
        if (args->input_file_name[0] != 0 && args->output_file_name[0] != 0) return ARGP_ERR_UNKNOWN;
        if (args->input_file_name[0] == 0) strcpy(args->input_file_name, arg);
        else strcpy(args->output_file_name, arg);
        break;
    case ARGP_KEY_END:
        if (args->input_file_name[0] == 0 || args->output_file_name[0] == 0) 
        {
            argp_error(state, "Too few arguments");
            return ARGP_ERR_UNKNOWN;
        }
        break;
    default: 
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static void parse_args(int argc, char** argv, AsmArgs* args)
{
    const char* argp_program_version = "asm 1.0";
    const char* argp_program_bug_address = "jovanovicn.96@gmail.com";
    static char* doc = "Your program description.";
    static char* args_doc = "INPUTFILE OUTPUTFILE";
    static struct argp_option options[] = { 
        { "silent", 's', 0, 0, "Do not print anything."},
        { "verbose", 'v', 0, 0, "Print detailed info about progress."},
        { 0 } 
    };
    struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

    argp_parse(&argp, argc, argv, 0, 0, args);
}

int main(int argc, char** argv)
{
    AsmArgs args;
    FILE* input_file;
    FILE* output_file;
    Program* prog = 0;

    parse_args(argc, argv, &args);
    
    {
        printf("verb mode: ");
        switch (args.verb)
        {
        case ARGS_VERB_SILENT: printf("silent\n"); break;
        case ARGS_VERB_NORMAL: printf("normal\n"); break;
        case ARGS_VERB_VERBOSE: printf("verbose\n"); break;
        }
        printf("in file: %s\nout file %s\n", args.input_file_name, args.output_file_name);
    }

    input_file = fopen(args.input_file_name, "r");
    if (!input_file)
    {
        if (args.verb != ARGS_VERB_SILENT)
        {
            printf("Error opening input file %s : %s\n", args.input_file_name, strerror(errno));
        }
        exit(1);
    }
    fclose(input_file);
    output_file = fopen(args.output_file_name, "w");
    if (!output_file)
    {
        if (args.verb != ARGS_VERB_SILENT)
        {
            printf("Error opening output file %s : %s\n", args.output_file_name, strerror(errno));
        }
        exit(1);
    }
    fclose(output_file);

    if (args.verb == ARGS_VERB_VERBOSE) printf("Assembling started.\n");

    prog =  new_program();
    input_file = fopen(args.input_file_name, "r");

    //getline(input_file);

    fclose(input_file);

    switch (prog_store(prog, args.output_file_name))
    {
    case PROG_RET_SUCCESS:
        if (args.verb == ARGS_VERB_VERBOSE) printf("Program saved.\n");
        break;
    case PROG_RET_INVALID_PATH:
        if (args.verb != ARGS_VERB_SILENT) printf("Unable to open output file.\n");
        break;
    case PROG_ERT_INVALID_PROGRAM:
        if (args.verb != ARGS_VERB_SILENT) printf("Could not save invalid program.\n");
        break;
    }

    free(prog);
    prog = 0;

    return 0;
}
