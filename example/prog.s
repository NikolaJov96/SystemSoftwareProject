; Main loop - Printing chars and testing instructions
; Example assembly command from root folder:
;   /asm -v -a 1024 example/prog.s example/prog.o
; After assembling all three example files link theum using for example:
;   ./emu -vr -b example/prog1 -l example/prog1.o example/prog.o example/prog_d.o example/prog_startup.o

; Debug enable/disable command:         
;       or      psw     16384
;       and     psw     -16385

.global start, numc, chra, chrsp, chrnl
.text
start: 
        ; loop
loop:   mov     *65534  r0
        push    r0
        add     r0      1
        add     r1      1
        cmp     numc    r1
        jmpgt   loop
        ; allow interrupts
        or      psw     32768
        ; second loop
lool:   pop     r0
        mov     *65534  r0
        sub     r1      1
        cmp     r1      0
        jmpgt   $lool
        mov     r2      16
        mov     *65534  r2
        ; write 0 to shared address
        mov     *16448  r1
        ; wait for timer to reset flag
wait:   cmp     r1      *16448
        jmpeq   $wait
        mov     *65534  r2
        
        ; r0 is 'A', set r1 to 'B', set r2 to '\n', r4 to ' '
        mov     r1      r0
        add     r1      1
        mov     r2      chrnl
        mov     r4      chrsp

        ; check all instructions

        ; check add 1
        call    chadd
        mov     *65534  r2

        ; check sub 2
        call    $chsub
        mov     *65534  r2

        ; check mul 3
        mov     r3      &chmul
        call    r3
        mov     *65534  r2

        ; check div 4
        mov     r3      10
        div     r3      5
        cmp     r3      2
        moveq   *65534  r0
        cmp     r3      2
        movne   *65534  r1
        mov     *65534  r2

        ; check and 5
        mov     r3      1365
        and     r3      127
        cmp     r3      85
        moveq   *65534  r0
        cmp     r3      85
        movne   *65534  r1
        mov     *65534  r2

        ; check or 6
        mov     r3      -28671
        or      r3      28673
        cmp     r3      -4095
        moveq   *65534  r0
        cmp     r3      -4095
        movne   *65534  r1
        mov     *65534  r2

        ; check not 7
        mov     r3      255
        not     r3      r3
        cmp     r3      -256
        moveq   *65534  r0
        cmp     r3      -256
        movne   *65534  r1
        mov     *65534  r2

        ; check test 8
        mov     r3      85
        test    r3      170
        moveq   *65534  r0
        test    r3      170
        movne   *65534  r1
        mov     *65534  r2

        ; check shr 9
        mov     r3      -3840
        shl     r3      2
        cmp     r3      -15360
        moveq   *65534  r0
        cmp     r3      -15360
        movne   *65534  r1
        mov     *65534  r2

        ; chech shl 10
        mov     r3      143
        shr     r3      2
        cmp     r3      35
        moveq   *65534  r0
        cmp     r3      35
        movne   *65534  r1
        mov     *65534  r2

.word   0

chadd:  mov     r3      5
        add     r3      5
        cmp     r3      10
        moveq   *65534  r0
        cmp     r3      10
        movne   *65534  r1

        mov     r3      5
        add     r3      5
        push    0
        call    $flagn
        pop     r3
        
        mov     r3      -5
        add     r3      -5
        push    1
        call    $flagn
        pop     r3
        
        mov     r3      32767
        add     r3      1
        push    1
        call    $flago
        pop     r3

        mov     r3      32767
        add     r3      10
        push    0
        call    $flagc
        pop     r3

        mov     r3      -1
        add     r3      1
        push    1
        call    $flagc
        pop     r3

        ret

chsub:  mov     r3      -10
        sub     r3      -5
        cmp     r3      -5
        moveq   *65534  r0
        cmp     r3      -5
        movne   *65534  r1
        
        mov     r3      30000
        sub     r3      -30000
        push    1
        call    $flagn
        pop     r3

        mov     r3      30000
        sub     r3      -30000
        push    1
        call    $flagn
        pop     r3

        ret

chmul:  mov     r3      -7
        mul     r3      -3
        cmp     r3      21
        moveq   *65534  r0
        cmp     r3      21
        movne   *65534  r1
        ret

; flag chackers

flagn:  mov     r3      psw
        shr     r3      3
        and     r3      1
        mov     r4      r6[2]
        cmp     r3      r4
        moveq   *65534  r0 
        cmp     r3      r4
        movne   *65534  r1
        ret

flagc:  mov     r3      psw
        shr     r3      2
        and     r3      1
        mov     r4      r6[2]
        cmp     r3      r4
        moveq   *65534  r0 
        cmp     r3      r4
        movne   *65534  r1
        ret

flago:  mov     r3      psw
        shr     r3      1
        and     r3      1
        mov     r4      r6[2]
        cmp     r3      r4
        moveq   *65534  r0 
        cmp     r3      r4
        movne   *65534  r1
        ret

flagz:  mov     r3      psw
        and     r3      1
        mov     r4      r6[2]
        cmp     r3      r4
        moveq   *65534  r0 
        cmp     r3      r4
        movne   *65534  r1
        ret

.data
        ;.align  2
        ;.skip   1
        ;.align  2
chra:   .word   65
chrsp:  .word   32

.end
