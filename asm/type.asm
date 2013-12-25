            .ORG 0
            MOV %sp, 0x30000    ; Sets Stack Pointer to the end of the 128KiB RAM

            MOV %r0, 0
            STORE.B 0xFF0ACC00, %r0 ; Text mode 0, default font and palette, no vsync

            MOV %r0, 0xFF0A0000
            MOV %r1, 0x70
            CALL clr_screen

            MOV %r10, 0xFF0A0000 ; %r10 stores screen pointer
            MOV %r11, 0 ; Row
            MOV %r12, 0 ; Col
loop:       
            LOAD.W %r0, 0xFF000060  ; Reads KEY_REG
            IFEQ %r0, 0
              RJMP loop

            IFCLEAR %r0, 0x100  ; Ignores Key Up
              RJMP loop
            AND %r1, %r0, 0xFF
           
            IFEQ %r1, 0x0D        ; Return Key
              RJMP return

            IFEQ %r1, 0x08        ; Backspace Key
              RJMP delete

            IFLE %r1, 0x20        ; Ignores not printable keys
              RJMP loop
              
            IFL %r1, 0x41
              RJMP paint_char
            
            IFLE %r1, 0x5A
              RCALL check_shift

paint_char:
            ; Computes position to write
            LLS %r13, %r12, 1
            MUL %r14, %r11, 80
            ADD %r13, %r13, %r14
            ADD %r13, %r13, %r10

            ; Types
            STORE.B %r13, %r1 
            ADD %r12, %r12, 1
            
            ; Increments cursor
            IFL %r12, 40
              RJMP loop
            MOV %r12, 0
            ADD %r11, %r11, 1


end_loop:
            RJMP loop


return:
            MOV %r12, 0
            ADD %r11, %r11, 1
            RJMP end_loop

delete:
            MOV %r0, 0x20 ; Space character
            SUB %r12, %r12, 1   ; Col -1
            IFSL %r12, 0  ; Is Negative? Then change row
              RJMP del_row

            ; Computes position to write
            LLS %r13, %r12, 1
            MUL %r14, %r11, 80
            ADD %r13, %r13, %r14
            ADD %r13, %r13, %r10

            ; Types
            STORE.B %r13, %r0 

            RJMP end_loop

del_row:
            MOV %r12, 0  ; Col 0
            SUB %r11, %r11, 1  ; Row -1
            IFSL %r11, 0   ; If Row is negative, coverts to 0
              MOV %r11, 0

            ; Computes position to write
            LLS %r13, %r12, 1
            MUL %r14, %r11, 80
            ADD %r13, %r13, %r14
            ADD %r13, %r13, %r10

            ; Types
            STORE.B %r13, %r0 

            RJMP end_loop

check_shift:
            IFBITS %r0, 0b1000000000  ; Shift being pressed
              RET

            ADD %r1, %r1, 32 ; Convert ascii to lowercase
            RET

            .include "lib.inc"

