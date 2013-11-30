            .ORG 0
            MOV %sp, 0x30000    ; Sets Stack Pointer to the end of the 128KiB RAM

            MOV %r0, 0
            STORE.B 0xFF0ACC00, %r0 ; Text mode 0, default font and palette, no vsync

            MOV %r0, 0xFF0A0000
            MOV %r1, 0x70
            CALL clr_screen

            MOV %r10, 0xFF0A0000 ; %r10 stores cursor position

loop:       
            LOAD.B %r0, 0xFF000060  ; Reads KEY_REG
            ;MOV %r0, 193
            IFEQ %r0, 0
              RJMP loop

            IFCLEAR %r0, 0b10000000  ; Igonres Key Up
              RJMP loop
            AND %r0, %r0, 0x7F
            
            ;IFEQ %r0, 0x08
            ;  JMP delete

            STORE.B %r10, %r0 
            ADD %r10, %r10, 2

end_loop:
            RJMP loop


            .include "lib.inc"

