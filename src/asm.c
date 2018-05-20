#include "asm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prog.h"

void preprocess_line(char* line)
{
    int len, i, j, acc;

    for (i = 0; line[i]; i++) if (line[i] == ';') { line[i] = 0; break; }

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

            label[i] = 0;

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
        Instruction ins;
        Directive dir;
        int ret;

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
            ret = prog_add_sym(prog, SYM_SECTION, line, acc_offset);
            if (ret != 0)
            {
                if (args.verb != ARGS_VERB_SILENT) 
                {
                    if (ret == 1) printf("Invalid label, line %d : %s\n", line_num, line);
                    else if (ret == 2) printf("Label already defined, line %d : %s\n", line_num, line);
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
            ret = prog_add_sym(prog, SYM_LABEL, label, acc_offset);
            if (ret != 0)
            {
                if (args.verb != ARGS_VERB_SILENT) 
                {
                    if (ret == 1) printf("Invalid label, line %d : %s\n", line_num, line);
                    else if (ret == 2) printf("Label already defined, line %d : %s\n", line_num, line);
                }
                fclose(input_file);
                exit(1);
            }
            else if (args.verb == ARGS_VERB_VERBOSE) printf("  Label: %s\n", label);
        }

        if (line[0] == 0) continue;

        ret = ins_parse(&ins, line);
        if (ret == 0)
        {
            int max_op_len = 0, op_len = 0, ind = 0;
            int addr_error = 0;
            InsOp* op;
            
            for (op = ins.ops_head; op; op = op->next)
            {
                if ((addr_error = ins_valid_addr(ins.ins, op->addr, ind)) != 0)
                {
                    if (args.verb != ARGS_VERB_SILENT) 
                    {
                        if (addr_error == 1)
                            printf("Invalid addressing for operand %d, line %d : %s\n", ind, line_num, line);
                        if (addr_error == 2) 
                            printf("Too many parameters, line %d : %s\n", line_num, line);
                    }
                    fclose(input_file);
                    exit(1);
                }
                op_len = ins_len(op->addr);
                if (op_len > max_op_len) max_op_len = op_len;
                else if (op_len == max_op_len && op_len == 4)
                {
                    if (args.verb != ARGS_VERB_SILENT) 
                    {
                        printf("Cannot use muliple immediate values in one instruction, line %d : %s\n", line_num, line);
                    }
                    fclose(input_file);
                    exit(1);
                }
                ind++;
            }

            acc_offset += max_op_len;
        }
        else if (ret != 2)
        {
            if (args.verb != ARGS_VERB_SILENT) 
            {
                switch (ret)
                {
                case 1: case 5: printf("Fatal instruction error, line %d : %s\n", line_num, line); break;
                case 3: printf("Invalid first operand, line %d : %s\n", line_num, line); break;
                case 4: printf("Invalid second operand, line %d : %s\n", line_num, line); break;
                }
            }
            fclose(input_file);
            exit(1);
        }
        ins_op_free(&ins);

        if (ret == 2)
        {
            ret = dir_parse(&dir, line);
            if (ret == 0)
            {
                int dir_args_len = dir_len(&dir, acc_offset);
                if (dir_args_len == -1)
                {
                    if (args.verb != ARGS_VERB_SILENT) 
                    {
                        printf("Invalid use of directive, line %d : %s\n", line_num, line);
                    }
                    fclose(input_file);
                    exit(1);
                }
                acc_offset += dir_args_len;
            }
            else
            {
                if (args.verb != ARGS_VERB_SILENT) 
                {
                    printf("Invalid line %d : %s\n", line_num, line);
                    printf("Ret: %d\n", ret);
                }
                fclose(input_file);
                exit(1);
            }
            dir_arg_free(&dir);
        }
    }

    fclose(input_file);

    if (args.verb == ARGS_VERB_VERBOSE) printf("Second pass started.\n");
    input_file = fopen(args.input_file_name, "r");
    line_num = 0;

    while (fgets(line, sizeof(line), input_file)) 
    {
        char label[50];
        Instruction ins;
        Directive dir;
        int ret;

        line_num++;
        preprocess_line(line);
        if (skip_line(line)) continue;
        if (parse_section(line) != SEC_NONE) continue;
        get_label(line, label);
        if (line[0] == 0) continue;

        ret = ins_parse(&ins, line);
        if (ret == 0)
        {
            char data[4] = { 0, 0, 0, 0 };
            int i, imm_value = 0;
            InsOp* op;

            switch (ins.cond)
            {
            case COND_EQ: data[0] = 0b00000000; break;
            case COND_NE: data[0] = 0b01000000; break;
            case COND_GT: data[0] = 0b10000000; break;
            case COND_AL: data[0] = 0b11000000; break;
            }

            switch (ins.ins)
            {
            case INS_ADD:  data[0] |= 0b00000000; break;
            case INS_SUB:  data[0] |= 0b00000100; break;
            case INS_MUL:  data[0] |= 0b00001000; break;
            case INS_DIV:  data[0] |= 0b00001100; break;
            case INS_CMP:  data[0] |= 0b00010000; break;
            case INS_AND:  data[0] |= 0b00010100; break;
            case INS_OR:   data[0] |= 0b00011000; break;
            case INS_NOT:  data[0] |= 0b00011100; break;
            case INS_TEST: data[0] |= 0b00100000; break;
            case INS_PUSH: data[0] |= 0b00100100; break;
            case INS_POP: case INS_RET:  data[0] |= 0b00101000; break;
            case INS_IRET: data[0] |= 0b00101100; break;
            case INS_CALL: data[0] |= 0b00110000; break;
            case INS_MOV:  data[0] |= 0b00110100; break;
            case INS_SHL:  data[0] |= 0b00111000; break;
            case INS_SHR:  data[0] |= 0b00111100; break;
            case INS_JMP:
                if (ins.ops_head->addr == ADDR_PCREL) data[0] |= 0b00000000;
                else data[0] |= 0b00110100;
            break;
            }
            
            for (op = ins.ops_head, i = 0; op; op = op->next, i++)
            {
                char byte = 0;
                if (op->addr == ADDR_PSW) byte = 0b00000111;
                else if (op->addr == ADDR_REGDIR) byte = 0b000001000 | (op->reg & 0b111);
                else
                {
                    imm_value = 1;

                    switch (op->addr)
                    {
                    case ADDR_IMM: byte = 0b0000000; break;
                    case ADDR_MEMDIR: byte = 0b00010000; break;
                    case ADDR_REGINDDISP: byte = 0b00011000 | (op->reg & 0b111); break;
                    case ADDR_PCREL: /*pcrel*/ break;
                    }

                    if (op->label[0] == 0) 
                    {
                        // check little endian order
                        data[2] = op->val >> 8;
                        data[3] = op->val & 0xFF;
                    }
                    else
                    {
                        // relocation
                        data[2] = 0;
                        data[3] = 0;
                    }
                }

                if (i == 0) { data[0] |= (byte >> 3) & 0b11; data[1] |= byte << 5; }
                else if (i == 1) { data[1] |= byte & 0b11111; }
            }
            printf("%d %d %d %d\n", data[0], data[1], data[2], data[3]);

            prog_add_data(prog, data[0]);
            prog_add_data(prog, data[1]);
            if (imm_value)
            {
                prog_add_data(prog, data[0]);
                prog_add_data(prog, data[0]);
            }
        }
        ins_op_free(&ins);
        if (ret == 2)
        {
            // sta je labela u direktivama
            // mov pc?
            // test
            ret = dir_parse(&dir, line);
            if (ret == 0)
            {
                long label, number, neg, bytes;
                int i;
                DirArg* arg;
                switch (dir.dir)
                {
                case DIR_CHAR: label = 0; number = 1; neg = 1; bytes = 1; break;
                case DIR_WORD: label = 1; number = 1; neg = 1; bytes = 2; break;
                case DIR_LONG: label = 1; number = 1; neg = 1; bytes = 4; break;
                case DIR_ALIGN: case DIR_SKIP: label = 0; number = 1; neg = 0; bytes = 2; break;
                case DIR_GLOBAL: label = 1; number = 0; neg = 0; bytes = 2; break;
                }
                for (arg = dir.args_head, i = 0; arg; arg = arg->next, i++)
                {
                    long abs_val = arg->val;
                    if (abs_val < 0) abs_val = -abs_val;
                    else abs_val++;
                    if (arg->label[0] && !label)
                    {
                        if (args.verb != ARGS_VERB_SILENT) 
                        {
                            printf("Cannot use label with this directive, line %d : %s\n", line_num, line);
                        }
                        fclose(input_file);
                        exit(1);
                    }
                    if (!arg->label[0] && !number)
                    {
                        if (args.verb != ARGS_VERB_SILENT) 
                        {
                            printf("Cannot use number with this directive, line %d : %s\n", line_num, line);
                        }
                        fclose(input_file);
                        exit(1);
                    }
                    if (arg->val < 0 && !neg)
                    {
                        if (args.verb != ARGS_VERB_SILENT) 
                        {
                            printf("Cannot use negative value with this directive, line %d : %s\n", line_num, line);
                        }
                        fclose(input_file);
                        exit(1);
                    }
                    if (abs_val > (1l << (bytes * 8 - 1)))
                    {
                        if (args.verb != ARGS_VERB_SILENT) 
                        {
                            printf("--%d\n", arg->val);
                            printf("Value %d too large, line %d : %s\n", i + 1, line_num, line);
                        }
                        fclose(input_file);
                        exit(1);
                    }
                }
            }
            dir_arg_free(&dir);
        }
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

    prog_free(&prog);

    return 0;
}
