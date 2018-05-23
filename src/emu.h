#ifndef EMU_H
#define EMU_H

#include <argp.h>

typedef enum { ARGS_VERB_SILENT, ARGS_VERB_NORMAL, ARGS_VERB_VERBOSE } ARGS_VERB;

typedef struct InputListNode
{
    char file[250];
    struct InputListNode* next;
} InputListNode;

typedef struct EmuArgs
{
    ARGS_VERB verb;
    InputListNode* in_head;
    InputListNode* in_tail;
    int do_link;
    char link_target[250];
    int do_build;
    char build_target[250];
    int do_run;
} EmuArgs;

void parse_args(int argc, char** argv, EmuArgs* args);

#endif  // EMU_H
