; Setting up interrupt routines, assemble with -a 0
; Example assembly command from root folder:
;   ./asm -v -a 0 example/prog_startup.s example/prog_startup.o

.global chra, chrnl, chrsp
.data
        .word stup, timer, invins, key
        .word 0, 0, 0, 0

.text
stup:   ; startup routine
        ; init state of r0 and r1 for the main loop
        mov     r0      chra
        mov     r1      0
        iret

        ; timer routine
timer:  push    r0      ; preserve r0
        mov     r0      1
        add     *16448  r0
        pop     r0
        iret

invins: mov     r0      73
        mov     *65534  r0
        or      psw     2048
        iret


        ; key pressed routine
key:    push    r0      ; preserve r0
        ; print space
        mov     r0      chrsp
        mov     *65534  r0
        ; print caps pressed char
        mov     r0      *65532
        mov     *65534  r0
        pop     r0
        iret

.end
