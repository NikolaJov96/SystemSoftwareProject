System software project
=======================

Assembler and emulator for hypothetical machine
----------------------------------------------

### Assembler

Taking assembly code and producing object file.

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

Used architecture is 16 bit, byte-addressable, using little-endian, addresable space is 2^16 B.

Stack grows towards lower addresses and stack pointer points to the last used location.

There are 8 16-bit registers, acting as signed in areithmetic operations. Register r6 represents stack pointer and register r7 represents program counter.

Program status register is additional 16-bit register:
- psw0 - zero flag
- psw1 - overflow flag
- psw2 - carry flag
- psw3 - negative flag
- psw11 - halt flag
- psw13 - 1s timer interrupt mask
- psw14 - print debug (registers anmd flags) after each instruction
- psw15 - global interrupt mask

Interrupt vector table contains 8 entries and begins on address 0:
- 0 - startup routine
- 2 - 1s timer routine
- 4 - invalid instruction routine
- 6 - key pressed interrupt routine
- 8, 10, 12 and 14 are free for programmer's use

Last 128 bytes are reserved for preiferies. Writting to the address 0xFFFE prints witten character to the termial. When a key is pressed, key value is placed on the address 0xFFFC and interrupt is generated.

### Assembly language

.align only has effect if assembled with -a option

Nikola Jovanovic
