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
<td>-s</td><td>--silent</td>       <td>Do not print anything</td>
</tr><tr>
<td>-v</td><td>--verbose</td>      <td>Print detailed info about progress</td>
</tr><tr>
<td>-?</td><td>--help</td>         <td>Give this help list</td>
</tr><tr>
<td></td>  <td>--usage</td>        <td>Give a short usage message</td>
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

Nikola Jovanovic
