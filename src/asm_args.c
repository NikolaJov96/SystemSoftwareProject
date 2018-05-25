#include "asm.h"

#include <stdlib.h>
#include <string.h>

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    AsmArgs* args = state->input;
    FILE* file;
    
    switch (key) {
    case ARGP_KEY_INIT:
        args->verb = ARGS_VERB_NORMAL;
        args->input_file_name[0] = 0;
        args->output_file_name[0] = 0;
        args->start_addr = -1;
        break;
    case 'a': 
        args->start_addr = atoi(arg);
        if (args->start_addr < 0 || (arg[0] != '0' && args->start_addr == 0))
        {
            argp_error(state, "Invalid starting address");
            return ARGP_ERR_UNKNOWN;
        }
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

void parse_args(int argc, char** argv, AsmArgs* args)
{
    const char* argp_program_version = "asm 1.0";
    const char* argp_program_bug_address = "jovanovicn.96@gmail.com";
    static char* doc = "Taking assembly code and producing object file.";
    static char* args_doc = "INPUT_FILE OUTPUT_FILE";
    static struct argp_option options[] = {
        { "address", 'a', "ADDR", 0, "Starting address of the file"       },
        { "verbose", 'v', 0,      0, "Print detailed info about progress" },
        { "silent",  's', 0,      0, "Do not print anything"              },
        { 0 } 
    };
    struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

    argp_parse(&argp, argc, argv, 0, 0, args);
}
