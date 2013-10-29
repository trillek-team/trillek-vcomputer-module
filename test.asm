begin:
        MOV %r0, 1
        MOV %r1, 1
        MOV %r2, 2 
        MOV %r3, 3 
        MOV %r4, 4 
        MOV %r5, 5 
        MOV %r6, 6 
        MOV %r7, 7 
        MOV %r8, 8 
        MOV %r9, 9 
        MOV %r10, 10 
        MOV %r11, 11
        MOV %r12, 12 
        MOV %r13, 13
        MOV %r14, 14
        MOV %r15, 15
        MOV %r16, 16
        MOV %r17, 17 
        MOV %r18, 18 
        MOV %r19, 19
        MOV %r20, 20
        MOV %r21, 21
        MOV %r22, 22
        MOV %r23, 23
        MOV %r24, 24
        MOV %r25, 25
        MOV %r26, 26
        MOV %r27, 27
        MOV %r28, 28
        MOV %r29, 29
        MOV %r30, 30
        MOV %r31, 31        ; Addr: 084h
        MOV %r30, 0        
        MOV %r31, 0        
        MOV %r1, 0xBEBECAFE   
        ; Tested seting registers and using bit literal

        MOV %r10 , %r1      ; Addr 090h
        NOT %r0, %r0
        XCHGB %r4
        XCHGW %r5

        MOV %r7, -1
        MOV %r6, -2
        ADD %r12, %r8, %r4 
        ADD %r0, %r8, 1 
        SUB %r22, %r22, %r20 
        SUB %r22, %r24, 4
        SUB %r22, %r24, -4

        MOV %r0 , 0         ; Addr 0C0h
loop:
        NOP                 ; Addr 0C4h
        ADD %r0, %r0, 1

        IFL %r0, 10         ; Addr 0CCh
            JMP loop            ; Jumps to 0C4h

end_loop:        
        MOV %sp, 0x20000    ; (Stack pointer to the last address of RAM)
    
        PUSH -13570         ; push 0xFFFFCAFE
        PUSH %r6
        POP  %r29
        POP  %r28           ; Addr 0E8h   %r29 = 0xFFFFCAFE
    
        MOV %r0, 2
        MOV %r1, 4
        CALL function_pow   ; Should return %r0 = 2^4 = 16

        JMP begin           ; Jumps to 0
table:  .DAT 0x55,0x1234,65,33,12
        .DAT 4,0xAA,2,0x12345678
        .DAT 0x00, 0x00, 0x00   ; Padding
        MOV %r11, 0x11


; Function naive interger power
; Params :
;  %r0 base
;  %r1 poitive exponent
; Return :
;  %r0 = base ^ exponent
function_pow:
        PUSH %r4
        PUSH %r5
        PUSH %y
        PUSH %flags

        MOV %r4, 0
        MOV %r5, %r0
        MOV %r0, 1              ; x^0 = 1
    
function_pow_beginloop:
        IFLE %r1, %r4            ; While %r1 >= %r4 (exponent >= counter)
            RJMP function_pow_endloop
        MUL %r0, %r0, %r5       ; %r0 = %r0 * base
        ADD %r4, %r4, 1         ; %r4++
        RJMP function_pow_beginloop

function_pow_endloop:

        POP %flags
        POP %y
        POP %r5
        POP %r4

        RET
