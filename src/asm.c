#include "asm.h"

#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>

#include "prog.h"

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    Arguments* args = state->input;
    switch (key) {
    case 'v':
        if (args->verb != ARGS_VERB_NORMAL) args->error = 1;
        else args->verb = ARGS_VERB_VERBOSE;
        break;
    case 's': 
        if (args->verb != ARGS_VERB_NORMAL) args->error = 1;
        else args->verb = ARGS_VERB_SILENT;
        break;
    case ARGP_KEY_ARG:
        if (strlen(arg) > 255) args->error = 1;
        else
        {
            StringListNode* newNode = malloc(sizeof(StringListNode));
            strcpy(newNode->str, arg);
            newNode->next = 0;
            if (args->input_files_head == 0)
            {
                args->input_files_head = newNode;
                args->input_files_tail = newNode;
            }
            else
            {
                args->input_files_tail->next = newNode;
                args->input_files_tail = newNode;
            }
        }
        break;
    default: 
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static void parse_args(int argc, char** argv, Arguments* args)
{
    const char* argp_program_version = "asm 1.0";
    const char* argp_program_bug_address = "jovanovicn.96@gmail.com";
    static char* doc = "Your program description.";
    static char* args_doc = "[FILENAME, ...]";
    static struct argp_option options[] = { 
        { "verbose", 'v', 0, 0, "Print detailed info about progress."},
        { "silent", 's', 0, 0, "Do not print anything."},
        { 0 } 
    };
    struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

    args->verb = ARGS_VERB_NORMAL;
    args->input_files_head = 0;
    args->input_files_tail = 0;
    args->error = 0;

    argp_parse(&argp, argc, argv, 0, 0, args);
}

static void free_args(Arguments* args)
{
    while (args->input_files_head != 0)
    {
        StringListNode* del = args->input_files_head;
        args->input_files_head = args->input_files_head->next;
        free(del);
    }
}

int main(int argc, char** argv)
{
    Arguments args;
    parse_args(argc, argv, &args);
    
    {
        printf("verb mode: ");
        switch (args.verb)
        {
        case ARGS_VERB_SILENT: printf("silent\n"); break;
        case ARGS_VERB_NORMAL: printf("normal\n"); break;
        case ARGS_VERB_VERBOSE: printf("verbose\n"); break;
        }
        printf("files:\n");
        for (StringListNode* node = args.input_files_head; node != 0; node = node->next)
            printf("  %s\n", node->str);
        printf("Error: %d\n", args.error);
    }

    if (args.error != 0)
    {
        if (args.verb != ARGS_VERB_SILENT) printf("Invalid arguments!\n");
        exit(1);
    }

    if (args.verb == ARGS_VERB_VERBOSE) printf("Assembling is starting.\n");

    StringListNode* file_name;
    for (file_name = args.input_files_head; file_name != 0; file_name = file_name->next)
    {
        if (args.verb != ARGS_VERB_SILENT) printf("Assembling file: %s.\n", file_name->str);

        Program* prog = new_program();

        free(prog);
        prog = 0;
    }

    free_args(&args);
    return 0;
}

