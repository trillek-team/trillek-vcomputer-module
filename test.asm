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
        MOV %r27, 27        ; This are special registers!
        MOV %r28, 28
        MOV %r29, 29
        MOV %r30, 30
        MOV %r31, 31        ; Addr: 07Ch
        MOV %bp, 0        
        MOV %sp, 0x30000    ; Sets Stack Pointer to the end of the 128KiB RAM     
        MOV %ia, 0                 
        MOV %flags, 0                 
        MOV %r1, 0xBEBECAFE   
        ; Tested seting registers and using bit literal

        MOV %r10 , %r1      ; %r10 = 0xBEBECAFE
        SWP %r7, %r9        ; %r9 = 7 ; %r7 = 9
        NOT %r0, %r0        ; %r0 = 0xFFFFFFFF = -1
        XCHGB %r4           ; %r4 = 0x400
        XCHGW %r5           ; %r5 = 0x50000

        ;Test IFx operations **************************************************
test_ifx:                       ; PC = 0x00B0
        IFEQ %r1, 0         ; %r1 = 0xBEBECAFE, so skips
            JMP crash
        IFNEQ %r1, 0xBEBECAFE ; " ", so skips
            JMP crash

        IFL %r6, %r2        ; 6 < 2 = false, so skips
            JMP crash
        IFLE %r6, %r2       ; 6 < 2 = false, so skips
            JMP crash

        IFSL %r2, %r0       ; 2 < -1 = false, so skips
            JMP crash
        IFSLE %r2, %r0      ; 2 < -1 = false, so skips
            JMP crash

        IFBITS %r4, 0x404   ; (0x400 & 0x404) != 0 so execute
            JMP next
        JMP crash

next:        
        IFCLEAR %r4, 0x404  ; (0x400 & 0x404) != 0 so skips
            JMP crash

        IFEQ %r1, 0         ; False, so must skip
            IFNEQ %r1, 0              ; True but is Chained and skiped
                IFEQ %r1, 0xBEBACAFE  ; True but is Chained and skiped
                    JMP crash         ; This never should be executed

        ; Test ALU operations *************************************************
test_alu:                       ; PC = 0x010C
        ; Testing BOOLEAN instructions
        MOV %r7, 0x5555AAAA
        MOV %r6, 0xAAFFFF55
        NOT %r12, %r6           ; %r12 = 0x550000AA
        IFNEQ %r12, 0x550000AA
            JMP crash

        AND %r12, %r7, %r6      ; %r12 = 0x0055AA00
        IFNEQ %r12, 0x0055AA00
            JMP crash

        OR %r12, %r7, %r6       ; %r12 = 0xFFFFFFFF
        IFNEQ %r12, 0xFFFFFFFF
            JMP crash

        XOR %r12, %r7, %r6      ; %r12 = 0xFFAA55FF
        IFNEQ %r12, 0xFFAA55FF
            JMP crash

        BITC %r12, %r7, %r6     ; %r12 = 0x550000AA
        IFNEQ %r12, 0x550000AA
            JMP crash

        ; Testing Addition/Substraction instructions
        ADD %r12, %r8, %r4      ; %r12 = %r8 + %r4 = 0x408
        IFNEQ %r12, 0x408
            JMP crash

        ADD %r0, %r8, 1         ; %r0 = %r8 +1 = 9
        IFNEQ %r0, 9
            JMP crash
        
        SUB %r22, %r12, %r0     ; %r22 = %r12 - %r0 = 0x3FF
        IFNEQ %r22, 0x3FF
            JMP crash

        SUB %r22, %r24, 4       ; %r22 = %r24 -4 = 20 = 0x14
        IFNEQ %r22, 20
            JMP crash

        SUB %r22, %r24, -4      ; %r22 = %r24 -(-4) = 28 = 0x1C
        IFNEQ %r22, 28
            JMP crash

        RSB %r22, %r24, -4      ; %r22 = -4 - %r24 = -28 = 0xFFFFFFE4
        IFNEQ %r22, -28
            JMP crash

        ; Testing Overflow
        MOV %r22, 0x40000000
        MOV %r23, 0x60000000
        ADD %r24, %r22, %r23    ; %r24 = 0x40000000 + 0x60000000 = 0xA0000000 (negative)
        IFCLEAR %flags, 2       ; If OF == 0 -> Jump to crash
            JMP crash
        ; TODO Overflow with substraction

        ; Testing Carry doing 64 bit addition and substraction
        MOV %r20, 0xFFFFFFFF    ; LSB op1
        MOV %r21, 0x00000001    ; MSB op1
        MOV %r22, 0x00000001    ; LSB op2
        MOV %r23, 0x00000100    ; MSB op2
        ; Result of addtion must be 0x00000102_00000000
        ADD %r24, %r20, %r22    ; Adds LSB
        ADDC %r25, %r21, %r23   ; Adds MSB
        IFNEQ %r24, 0
            JMP crash
        IFNEQ %r25, 0x102
            JMP crash
        
        ; Result of addtion must be 0xFFFFFF01_FFFFFFFE
        SUB %r24, %r20, %r22    ; Subs LSB
        SUBB %r25, %r21, %r23   ; Subs MSB
        IFNEQ %r24, 0xFFFFFFFE
            JMP crash
        IFNEQ %r25, 0xFFFFFF01
            JMP crash

        ; Testing Shift instructions
        ; %r6 = 0xAAFFFF55
        LLS %r12, %r6, 8        ; %r12 = 0xFFFF5500
        IFNEQ %r12, 0xFFFF5500
            JMP crash

        LRS %r12, %r6, 8        ; %r12 = 0x00AAFFFF
        IFNEQ %r12, 0x00AAFFFF
            JMP crash
        
        ARS %r12, %r6, 8        ; %r12 = 0xFFAAFFFF
        IFNEQ %r12, 0xFFAAFFFF
            JMP crash
        ARS %r12, %r7, 8        ; %r12 = 0x005555AA
        IFNEQ %r12, 0x005555AA
            JMP crash

        ROTL %r12, %r6, 8       ; %r12 = 0xFFFF55AA
        IFNEQ %r12, 0xFFFF55AA
            JMP crash

        ROTR %r12, %r6, 8       ; %r12 = 0x55AAFFFF
        IFNEQ %r12, 0x55AAFFFF
            JMP crash

        ; Testing Multiplication/Division
        MOV %r22, 100
        MOV %r23, 4000000001
        MUL %r24, %r23, %r22     ; %y:%r24 = 400000000100
        IFNEQ %r24, 0x21DBA064
            JMP crash
        IFNEQ %y, 0x5D
            JMP crash

        DIV %r24, %r23, %r22     ; %r24 = 40000000 ; %y = 1
        IFNEQ %r24, 0x02625A00
            JMP crash
        IFNEQ %y, 1
            JMP crash
        ; TODO Signed Multiplication/Division
        ; TODO Check Division error flag

        ; TODO Check LOAD/STORE
        ; TODO Check Stack instrucctions
        ; TODO Check CALL functions
        ; TODO Check other instrucctions

        CALL print_ok

        LOAD.B %r3, 0x10000     ; Load countervar
        
        ADD %r3, %r3, 1
        DIV %r4, %r3, 10
        MOV %r3, %y             ; %r3 = %r3 % 10

        STORE.B 0x10000, %r3    ; Stores counter new value TODO Fails
        
        ADD %r0, %r3, 0x30      ; + '0'
        MOV %r1, 0x0100         ; Column 0, Row 1
        MOV %r2, 0x70           ; Light gray paper, black Ink
        CALL print_chr


        
        JMP begin               ; Begin again in a endless loop
crash:
        SLEEP               ; Sleeps because something goes wrong


; *****************************************************************************
print_ok:                   ; Prints OK in CDA text mode 0
       MOV %r0, 0
       STORE.B 0xFF0ACC00, %r0 ; Text mode 0, default font and palette, no vsync
       
       MOV %r0, 0x0F6B0A4F
       STORE 0xFF0A0000, %r0   ; Writes Ok in VRRAM

       RET


; Prints a %r0 digit in screen 0 with textmode 0, at column %r1[0:7], row %r1[8:15]
;       attribute  %r2
; Asummes that row < 30 and column < 40
print_chr:
      PUSH %r4
      PUSH %r5
      AND %r4, %r1, 0x3F          ; Grabs column and puts in %r4
      LRS %r5, %r1, 8             ; Grabs row and puts in %r5
      MUL %r5, %r5, 0x28
      ADD %r4, %r4, %r5           ; %r4 = row*40 + column
      LLS %r4, %r4, 1             ; %r4 *= 2
      ADD %r4, %r4 ,0xFF0A0000
      STORE.B %r4, %r0            ; Write character
      ADD %r4, %r4, 1
      STORE.B %r4, %r2            ; Write Attribute

      POP %r5
      POP %r4

      RET


