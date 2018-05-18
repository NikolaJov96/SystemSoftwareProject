#include <argp.h>

typedef enum { ARGS_VERB_SILENT, ARGS_VERB_NORMAL, ARGS_VERB_VERBOSE } ARGS_VERB;

typedef struct
{
    ARGS_VERB verb;
    char input_file_name[256];
    char output_file_name[256];
} AsmArgs;

void parse_args(int argc, char** argv, AsmArgs* args);
