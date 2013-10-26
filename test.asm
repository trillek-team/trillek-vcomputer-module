begin:
        SET %r0, 1
        SET %r1, 1
        SET %r2, 2 
        SET %r3, 3 
        SET %r4, 4 
        SET %r5, 5 
        SET %r6, 6 
        SET %r7, 7 
        SET %r8, 8 
        SET %r9, 9 
        SET %r10, 10 
        SET %r11, 11
        SET %r12, 12 
        SET %r13, 13
        SET %r14, 14
        SET %r15, 15
        SET %r16, 16
        SET %r17, 17 
        SET %r18, 18 
        SET %r19, 19
        SET %r20, 20
        SET %r21, 21
        SET %r22, 22
        SET %r23, 23
        SET %r24, 24
        SET %r25, 25
        SET %r26, 26
        SET %r27, 27
        SET %r28, 28
        SET %r29, 29
        SET %r30, 30
        SET %r31, 31        ; Addr: 084h
        SET %r30, 0        
        SET %r31, 0        
        SET %r1, 0xBEBECAFE   
        ; Tested seting registers and using bit literal

        CPY %r10 , %r1      ; Addr 090h
        NOT %r0
        NEG %r3
        XCHG %r4
        XCHG.W %r5

        SET %r7, -1
        SET %r6, -2
        ADD %r12, %r8, %r4 
        ADD %r0, %r8, 1 
        SUB %r22, %r22, %r20 
        SUB %r22, %r24, 4
        SUB %r22, %r24, -4

        SET %r0 , 0         ; Addr 0C0h
loop:
        NOP                 ; Addr 0C4h
        ADD %r0, %r0, 1

        IFUG 10, %r0        ; Addr 0CCh
            JMP loop        ; Jumpts to 0C4h
        
        SET %sp, 0x20000    ; (Stack pointer to the last address of RAM)
    
        PUSH -13570         ; push 0xFFFFCAFE
        PUSH %r6
        POP  %r29
        POP  %r28           ; Addr 0E8h   %r29 = 0xFFFFCAFE
    
        JMP begin           ; Addr 0ECh   Jumps to 0

