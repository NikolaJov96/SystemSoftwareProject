; Some data used by main loop - linker test
; Example assembly command from root folder:
;   ./asm -v -a 2048 example/prog_d.s example/prog_d.o

.data
.global numc, chrnl
numc:   .word   26
chrnl:  .word   10

.end
