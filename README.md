System software project
=======================

Assembler and emulator for hypothetical machine
----------------------------------------------

### Installation

Just run make inside the project root folder.
```console
make
```

### Assembler

Taking assembly code and producing object file. Example program can be found in the /example folder.

Usage:
```console
asm [OPTION...] INPUT_FILE OUTPUT_FILE
```
Options:
<table><tr>
<td>-a</td><td>--address=ADDR</td><td>Starting address of the file</td>
</tr><tr>
<td>-s</td><td>--silent</td>      <td>Do not print anything</td>
</tr><tr>
<td>-v</td><td>--verbose</td>     <td>Print detailed info about progress</td>
</tr><tr>
<td>-?</td><td>--help</td>        <td>Give this help list</td>
</tr><tr>
<td></td>  <td>--usage</td>       <td>Give a short usage message</td>
</tr></table>

### Emulator

Taking multiple object files and linking them. Linked object file can be run or stored as object file or executable.

Usage:
```console
emu [OPTION...] [INPUT_FILE, ...]
```
Options:
<table><tr>
<td>-b</td><td>--build=OUTFILE</td><td>Link and build executable</td>
</tr><tr>
<td>-l</td><td>--link=OUTFILE</td><td>Save linked object file</td>
</tr><tr>
<td>-r</td><td>--run</td><td>Emulate linked program</td>
</tr><tr>
<td>-s</td><td>--silent</td><td>Do not print anything</td>
</tr><tr>
<td>-v</td><td>--verbose</td><td>Print detailed info about progress</td>
</tr><tr>
<td>-?</td><td>--help</td><td>Give this help list</td>
</tr><tr>
<td></td  ><td>--usage</td><td>Give a short usage message</td>
</tr></table>

### Architecture

Used architecture is 16 bit, byte-addressable, using little-endian, addressable space is 2^16 B.

Stack grows towards lower addresses and stack pointer points to the last used location, starting at "-128".

There are 8 16-bit registers, acting as signed in arithmetic operations. Register r6 represents stack pointer and register r7 represents program counter.

Program status register is additional 16-bit register:
- psw0 - zero flag
- psw1 - overflow flag
- psw2 - carry flag
- psw3 - negative flag
- psw11 - halt flag
- psw13 - 1s timer interrupt mask
- psw14 - print debug (registers and flags) after each instruction
- psw15 - global interrupt mask

Interrupt vector table contains 8 entries and begins on address 0:
- 0 - startup routine
- 2 - 1s timer routine
- 4 - invalid instruction routine
- 6 - key pressed interrupt routine
- 8, 10, 12 and 14 are free for programmer's use

Writing to the address 0xFFFE prints written character to the terminal. When a key is pressed, key value is placed on the address 0xFFFC and interrupt is generated.

All instructions are 2B and may contain additional 2B value, depending on operand addressing. Operands are interpreted as signed values.

Instruction encoding:
- bits 15-14: condition for instruction execution:
  - 00: equal - if zero flag is set
  - 01: not equal - if zero flag is not set
  - 10: greater - if not zero and not negative
  - 11: unconditional execution
- bits 13-10: instruction operation code
- bits 9-5: operand 1 (dst) encoding
- bits 4-0: operand 2 (src) encoding

Available instructions:
<table><tr>
<td>Ins</td> <td>Oc</td><td>Effect</td>                <td>Flags</td></tr><tr>
<td>add</td> <td>0</td> <td>dst = dst add src</td>     <td>zocn</td> </tr><tr>
<td>sub</td> <td>1</td> <td>dst = dst sub src</td>     <td>zocn</td> </tr><tr>
<td>mul</td> <td>2</td> <td>dst = dst mul src</td>     <td>zn</td>   </tr><tr>
<td>div</td> <td>3</td> <td>dst = dst div src</td>     <td>zn</td>   </tr><tr>
<td>cmp</td> <td>4</td> <td>dst sub src</td>           <td>zocn</td> </tr><tr>
<td>and</td> <td>5</td> <td>dst = dst & src</td>       <td>zn</td>   </tr><tr>
<td>or</td>  <td>6</td> <td>dst = dst | src</td>       <td>zn</td>   </tr><tr>
<td>not</td> <td>7</td> <td>dst = ~src</td>            <td>zn</td>   </tr><tr>
<td>test</td><td>8</td> <td>dst & src</td>             <td>zn</td>   </tr><tr>
<td>push</td><td>9</td> <td>sp -= 2; mem[sp] = src</td><td>-</td>    </tr><tr>
<td>pop</td> <td>10</td><td>dst = mem[sp]; sp += 2</td><td>-</td>    </tr><tr>
<td>call</td><td>11</td><td>push pc; pc = src</td>     <td>-</td>    </tr><tr>
<td>iret</td><td>12</td><td>pop psw; pop pc</td>       <td>-</td>    </tr><tr>
<td>mov</td> <td>13</td><td>dst = src</td>             <td>zn</td>   </tr><tr>
<td>shl</td> <td>14</td><td>dst = dst << src</td>      <td>zcn</td>  </tr><tr>
<td>shr</td> <td>15</td><td>dst = dst >> src</td>      <td>zcn</td>  </tr>
</table>
(Instructions ret and jmp are available in the assembler but are translated using some of the instructions above).

Operand encoding:
- bits 4-0 = 00111: access to the psw register
- other types:
  - immediate: 00|??? + 2B (value)
  - register direct: 01|reg
  - memory direct: 10|??? + 2B (memory address)
  - register indirect with displacement: 11|reg + 2B (displacement)

### Assembly language

Assembler features two pass implementation. First pass populates symbol table and checks for syntax errors. Second pass generates section data by translating instructions and populates relocation table.

Assembly input is one text file with the assembly code and the output is textual object file without any relocations resolved. Format of the output file is closely resembling elf format.

Language description:
- When assembling absolute file starting address can be specified as a command line argument, or it can be left to the linker 
- There is one command per line
- Each line can start with the label, consisting of letters followed by ':'
- Symbols are imported and exported using .global directive
- Each assembly file can consist of sections (max one of each):
  - .text - for program code
  - .data - for initialized data
  - .rodata - for initialized constant data
  - .bss - for uninitialized data
- Assembler does not distinguish between sections
- End of file is marked using .end
- Additional available directives:
  - .char - fills one byte with the explicit argument number
  - .word - fills two bytes with the explicit argument number or argument label address
  - .long - fills four bytes with the explicit argument number
  - .align - inserts zero bytes until absolute address dividable by the 2^argument is reached, has effect only if starting address is specified
  - .skip - inserts argument number of zero bytes
- Directives can have multiple arguments separated by comma
- Comments are indicated by ';'
- sp and pc are synonyms for r6 and r7

Operand syntax:
- 20 - immediate value
- &x - immediate value of the label address
- x - value form the address labeled x
- *20 - value from the address 20
- r3 - register direct addressing
- r4[32] - register indirect addressing with displacement (value from address r4 + 32)
- r4[x] - register indirect addressing with displacement (value from address r4 + &x)
- $x - pc relative addressing, equivalent to the pc[y] | pc + y = x

Additional instructions:
- ret - translates to pop pc, returning after a call
- jmp - jumps to the address, translates to:
  - add pc -displacement-; for pc relative addressing
  - mov pc -address-; for other addressing types
  - register indirect addressing with displacement cannot be implemented using one instruction

### Linker

When emulator is run, linker is called first. It resolves relocations inside each specified file, links files and at the end resolves global relocations.

Emulator (linker) can be asked to link provided files and save the linked object file and it can additionally run (emulate) the program if it has all relocations resolved and start label defined.

Emulator (linker) can also generate standalone executable file of the linked program. This is done by generating special main function that has all the linked code built in and calls the emulator routine. This function is stored in the file and linked with the code of the original emulator using gcc. The product is standalone executable of the emulator, with the built-in emulation machine code. 

For standalone generation to work, emulator (linker) must be run from the folder together with the necessary source files (used to make it), and also have access to the gcc command.

Nikola Jovanovic
