            .ORG 0
            MOV %sp, 0x30000    ; Sets Stack Pointer to the end of the 128KiB RAM

            MOV %r0, 0
            STORE.B 0xFF0ACC00, %r0 ; Text mode 0, default font and palette, no vsync

            MOV %r0, 0xFF0A0000
            MOV %r1, 0x70
            CALL clr_screen

            MOV %r10, 0xFF0A0000 ; %r10 stores screen pointer
            MOV %r8, 0 ; Row
            MOV %r9, 0 ; Col
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

            IFL %r1, 0x20         ; Ignores not printable keys
              RJMP loop
              
            IFL %r1, 0x2F
              RJMP paint_char
            
            IFLE %r1, 0x39        ; Number keys
              RCALL check_numbers
            
            IFL %r1, 0x41
              RJMP paint_char
            
            IFLE %r1, 0x5A
              RCALL check_shift   ; alphabetic keys

paint_char:
            ; Computes position to write
            LLS %r7, %r9, 1
            MUL %r6, %r8, 80
            ADD %r7, %r7, %r6
            ADD %r7, %r7, %r10

            ; Types
            STORE.B %r7, %r1 
            ADD %r9, %r9, 1
            
            ; Increments cursor
            IFL %r9, 40
              RJMP loop
            MOV %r9, 0
            ADD %r8, %r8, 1


end_loop:
            RJMP loop


return:
            MOV %r9, 0
            ADD %r8, %r8, 1
            RJMP end_loop

delete:
            MOV %r0, 0x20 ; Space character
            SUB %r9, %r9, 1   ; Col -1
            IFSL %r9, 0  ; Is Negative? Then change row
              RJMP del_row

            ; Computes position to write
            LLS %r7, %r9, 1
            MUL %r6, %r8, 80
            ADD %r7, %r7, %r6
            ADD %r7, %r7, %r10

            ; Types
            STORE.B %r7, %r0 

            RJMP end_loop

del_row:
            MOV %r9, 0  ; Col 0
            SUB %r8, %r8, 1  ; Row -1
            IFSL %r8, 0   ; If Row is negative, coverts to 0
              MOV %r8, 0

            ; Computes position to write
            LLS %r7, %r9, 1
            MUL %r6, %r8, 80
            ADD %r7, %r7, %r6
            ADD %r7, %r7, %r10

            ; Types
            STORE.B %r7, %r0 

            RJMP end_loop

check_shift:
            IFBITS %r0, 0b1000000000  ; Shift being pressed
              RET

            ADD %r1, %r1, 32 ; Convert ascii to lowercase
            RET

check_numbers:
            IFCLEAR %r0, 0b1000000000  ; Shift being not pressed
              RET

            IFEQ %r1, 0x31  ; Shift + 1
              RJMP shift1
            IFEQ %r1, 0x32  ; Shift + 2
              RJMP shift2
            IFEQ %r1, 0x33  ; Shift + 3
              RJMP shift3
            IFEQ %r1, 0x34  ; Shift + 4
              RJMP shift4
            IFEQ %r1, 0x35  ; Shift + 5
              RJMP shift5
            IFEQ %r1, 0x36  ; Shift + 6
              RJMP shift6
            IFEQ %r1, 0x37  ; Shift + 7
              RJMP shift7
            IFEQ %r1, 0x38  ; Shift + 8
              RJMP shift8
            IFEQ %r1, 0x39  ; Shift + 9
              RJMP shift9
            IFEQ %r1, 0x30  ; Shift + 0
              RJMP shift0
            RET

shift1:
            MOV %r1, '!'
            RET

shift2:
            MOV %r1, 0x40 ; @
            RET

shift3:
            MOV %r1, '#'
            RET

shift4:
            MOV %r1, '$'
            RET

shift5:
            MOV %r1, '%'
            RET

shift6:
            MOV %r1, '^'
            RET

shift7:
            MOV %r1, '&'
            RET

shift8:
            MOV %r1, '*'
            RET

shift9:
            MOV %r1, '('
            RET

shift0:
            MOV %r1, ')'
            RET








            .include "lib.inc"

