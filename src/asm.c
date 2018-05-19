#include "asm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prog.h"

void preprocess_line(char* line)
{
    int len, i, j, acc;
    len = strlen(line);
    if (len == 0) return;

    if (line[len - 1] == '\n') line[--len] = 0;
    for (i = 0; i < len; i++) if (line[i] == '\t') line[i] = ' ';
    
    acc = 0;
    for (i = len - 1; i >= -1; i--)
    {
        if (i >= 0 && line[i] == ' ') acc++;
        if (i < 0 || line[i] != ' ')
        {
            if (acc > 1)
            {
                for (j = i + 2 + acc - 1; j < len; j++) line[j - acc + 1] = line[j];
                len -= (acc - 1);
                line[len] = 0;
            }
            acc = 0;
        }
    }
    
    if (line[len - 1] == ' ') line[--len] = 0;
    if (line[0] == ' ')
    {
        for (i = 0; i < len; i++) line[i] = line[i+1];
        len--;
    }
    
    for (i = 0; i < len; i++) if (line[i] >= 'A' && line[i] <= 'Z') line[i] += 'a' - 'A';
}

int skip_line(char* line)
{
    if (line[0] == 0) return 1;
    return 0;
}

SECTION parse_section(char* line)
{
    if (strcmp(line, ".text") == 0) return SEC_TEXT;
    if (strcmp(line, ".data") == 0) return SEC_DATA;
    if (strcmp(line, ".rodata") == 0) return SEC_RODATA;
    if (strcmp(line, ".bss") == 0) return SEC_BSS;
    if (strcmp(line, ".end") == 0) return SEC_END;
    return SEC_NONE;
}

int get_label(char* line, char* label)
{
    int i;
    label[0] = 0;
    for (i = 0; line[i] != 0; i++)
    {
        if (line[i] >= 'a' && line[i] <= 'z') label[i] = line[i];
        else if (line[i] == ':') 
        {
            int ind;
            if (i >= 50) return 0;

            i++;
            if (line[i] == ' ') i++;
            ind = i;
            while (line[ind])
            {
                line[ind - i] = line[ind];
                ind++;
            }
            line[ind - i] = 0;
            return 1;
        }
        else break;
    }
    label[0] = 0;
    return 1;
}

int main(int argc, char** argv)
{
    AsmArgs args;
    FILE* input_file;
    FILE* output_file;
    Program* prog = 0;
    int line_num, acc_offset;
    char line[256];
    SECTION curr_section = SEC_NONE;
    int section_used[6] = { 1, 0, 0, 0, 0, 0 };

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

    prog = new_program();

    if (args.verb == ARGS_VERB_VERBOSE) printf("First pass started.\n");
    input_file = fopen(args.input_file_name, "r");
    line_num = 0;
    acc_offset = 0;

    while (fgets(line, sizeof(line), input_file)) 
    {
        SECTION new_sec;
        char label[50];

        line_num++;
        if (strlen(line) >= sizeof(line) - 1)
        {
            if (args.verb != ARGS_VERB_SILENT) 
            {
                line[40] = 0;
                printf("Line %d too long : %s...\n", line_num, line);
            }
            fclose(input_file);
            exit(1);
        }
        preprocess_line(line);
        
        if (skip_line(line)) continue;

        new_sec = parse_section(line);
        if (new_sec != SEC_NONE)
        {
            int status;
            if (section_used[new_sec]++ > 0)
            {
                if (args.verb != ARGS_VERB_SILENT) 
                {
                    printf("Already used section, line %d : %s\n", line_num, line);
                }
                fclose(input_file);
                exit(1);
            }
            if (args.verb == ARGS_VERB_VERBOSE)
            {
                switch (new_sec)
                {
                case SEC_TEXT: printf("  Section: .text\n"); break;
                case SEC_DATA: printf("  Section: .data\n"); break;
                case SEC_RODATA: printf("  Section: .rodata\n"); break;
                case SEC_BSS: printf("  Section: .bss\n"); break;
                case SEC_END: printf("  Section: .end\n"); break;
                }
            }
            curr_section = new_sec;
            if (new_sec == SEC_END) break;
            acc_offset = 0;
            if (!prog_add_sym(prog, SYM_SECTION, line, acc_offset))
            {
                if (args.verb != ARGS_VERB_SILENT) 
                {
                    printf("Invalid label, line %d : %s\n", line_num, line);
                }
                fclose(input_file);
                exit(1);
            }
            continue;
        }

        if (curr_section == SEC_NONE)
        {
            if (args.verb != ARGS_VERB_SILENT) 
            {
                printf("Unknown section on line %d : %s\n", line_num, line);
            }
            fclose(input_file);
            exit(1);
        }

        if (!get_label(line, label))
        {
            if (args.verb != ARGS_VERB_SILENT) 
            {
                line[40] = 0;
                printf("Invalid label, line %d : %s...\n", line_num, line);
            }
            fclose(input_file);
            exit(1);
        }
        if (label[0] != 0)
        {
            if (!prog_add_sym(prog, SYM_LABEL, label, acc_offset))
            {
                if (args.verb != ARGS_VERB_SILENT) 
                {
                    printf("Invalid label, line %d : %s\n", line_num, line);
                }
                fclose(input_file);
                exit(1);
            }
            else if (args.verb == ARGS_VERB_VERBOSE) printf("  Label: %s\n", label);
        }

        // parse instruction just to check if it has length of 2 or 4
        // acc_offset += 2
    }

    fclose(input_file);

    if (args.verb == ARGS_VERB_VERBOSE) printf("Second pass started.\n");
    input_file = fopen(args.input_file_name, "r");
    line_num = 0;

    while (fgets(line, sizeof(line), input_file)) 
    {
        line_num++;
    }

    fclose(input_file);
    if (args.verb == ARGS_VERB_VERBOSE) printf("Assembly finished.\n");

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
