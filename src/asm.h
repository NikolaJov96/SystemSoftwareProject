typedef enum { ARGS_VERB_SILENT, ARGS_VERB_NORMAL, ARGS_VERB_VERBOSE } ARGS_VERB;

typedef struct StringListNode
{
    char str[256];
    struct StringListNode* next;
} StringListNode;

typedef struct
{
    ARGS_VERB verb;
    StringListNode* input_files_head;
    StringListNode* input_files_tail;
    int error;
} Arguments;
