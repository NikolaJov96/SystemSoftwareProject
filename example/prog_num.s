; routine that accepts one argument and ptints it as a hex
; Example assembly command from root folder:
;   ./asm -v -a 3072 example/prog_num.s example/prog_num.o

.global prnum, chrnl, chra

.text
prnum:  push    r0
        push    r1
        push    r2

        mov     r0      sp[8]
        mov     r2      16

loop:   sub     r2      4
        mov     r1      r0
        shr     r1      r2
        and     r1      15
        cmp     r1      9
        jmp     letter
        addgt   r1      zero
        jmp     fend

letter: sub     r1      10
        add     r1      chra
    
fend:   mov     *65534  r1
        cmp     r2      0
        jmpgt   loop

        mov     r0      chrnl
        mov     *65534  r0

        pop     r2
        pop     r1
        pop     r0
        ret

.rodata
zero:   .word 48

.end
