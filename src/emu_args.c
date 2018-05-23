#include "emu.h"

#include <stdlib.h>
#include <string.h>

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    EmuArgs* args = state->input;
    FILE* file;
    InputListNode* in_node;
    
    switch (key) {
    case ARGP_KEY_INIT:
        args->verb = ARGS_VERB_NORMAL;
        args->in_head = 0;
        args->in_tail = 0;
        args->do_link = 0;
        args->do_build = 0;
        args->do_run = 0;
        break;
    case 'l': args->do_link = 1; strcpy(args->link_target, arg); break;
    case 'b': args->do_build = 1; strcpy(args->build_target, arg); break;
    case 'r': args->do_run = 1; break;
    case 'v':
        if (args->verb != ARGS_VERB_NORMAL) return ARGP_ERR_UNKNOWN;
        else args->verb = ARGS_VERB_VERBOSE;
        break;
    case 's': 
        if (args->verb != ARGS_VERB_NORMAL) return ARGP_ERR_UNKNOWN;
        else args->verb = ARGS_VERB_SILENT;
        break;
    case ARGP_KEY_ARG:
        in_node = malloc(sizeof(InputListNode));
        strcpy(in_node->file, arg);
        if (!args->in_head) args->in_head = in_node;
        else args->in_tail->next = in_node;
        args->in_tail = in_node;
        break;
    case ARGP_KEY_END:
        if (!args->in_head)
        {
            argp_error(state, "No imput files.");
            return ARGP_ERR_UNKNOWN;
        }
        if (!args->do_link && !args->do_build && !args->do_run)
        {
            argp_error(state, "Mode not specified.");
            return ARGP_ERR_UNKNOWN;
        }
        break;
    default: 
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

void parse_args(int argc, char** argv, EmuArgs* args)
{
    const char* argp_program_version = "emu 1.0";
    const char* argp_program_bug_address = "jovanovicn.96@gmail.com";
    static char* doc = "Your program description.";
    static char* args_doc = "[INPUTFILE, ...]";
    static struct argp_option options[] = {
        { "link",    'l', "OUTFILE", 0, "Save linked object file." },
        { "build",   'b', "OUTFILE", 0, "Link and build executable." },
        { "run",     'r', 0,         0, "Emulate linked program." },
        { "verbose", 'v', 0,         0, "Print detailed info about progress." },
        { "silent",  's', 0,         0, "Do not print anything." },
        { 0 } 
    };
    struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

    argp_parse(&argp, argc, argv, 0, 0, args);
}
