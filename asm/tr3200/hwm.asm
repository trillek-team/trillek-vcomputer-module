        .ORG 0
        MOV %sp, 0x30000    ; Sets Stack Pointer to the end of the 128KiB RAM     

        ; Setup screen
        MOV %r0, 0x80
        STORE.B 0xFF0ACC00, %r0 ; Text mode 0, default font and palette, no vsync

        MOV %r0, 0xFF0A0000
        MOV %r1, 0x70
        CALL clr_screen  ; Clear the screen

        ; Print "Nº Dev :"
        MOV %r0, str01
        MOV %r1, 0xFF0A0000
        MOV %r2, 0x70
        CALL print

        ; Ask how many devices are
        MOV %r0, 0
        STORE.W 0xFF000000, %r0 ; GET-NUMBER command
        LOAD.W %r10, 0xFF000000  ; Read value

        ; Print first digit
        DIV %r0, %r10, 10
        ADD %r0, %r0, '0'
        MOV %r1, 8
        MOV %r2, 0x70
        CALL print_chr
        
        ; Print second digit
        ADD %r0, %y, '0'
        MOV %r1, 9
        CALL print_chr
       
        ; Print Header
        MOV %r0, str02
        MOV %r1, 0xFF0A0050 ; Second row
        MOV %r2, 0x70
        CALL print
        
        MOV %r0, str03
        MOV %r1, 0xFF0A00A0 ; Third row
        MOV %r2, 0x70
        CALL print

        ; TODO Ask for the 32 posible devices and print a list of connected devices
        ; %r10 as device counter
        ; %r11 as screen pointer
        MOV %r10, -1
        MOV %r1, 0xFF0A00F0 ; Forth row

loop_dev:
        ADD %r10, %r10, 1
        IFL %r10, 32          ; if >= 32 then finish
          RJMP loop_dev_cont
        RJMP loop_dev_end

loop_dev_cont:
        ; 1 Ask Dev class
        OR %r3, %r10, 0x0100
        STORE.W 0xFF000000, %r3
        LOAD.B %r3, 0xFF000000
        IFEQ %r3, 0
          RJMP loop_dev       ; Not atached, so skips

        ; Print Dev Slot
        MOV %r0, %r10
        MOV %r2, 0x70
        CALL print_hex_b

        ; Print Dev Class
        MOV %r0, %r3
        ADD %r1, %r1, 4
        MOV %r2, 0x70
        CALL print_hex_b

        ; Ask Builder
        OR %r3, %r10, 0x0200
        STORE.W 0xFF000000, %r3
        LOAD.W %r0, 0xFF000000

        ; Print Dev Builder
        ADD %r1, %r1, 6
        MOV %r2, 0x70
        CALL print_hex_w

        ; Ask ID
        OR %r3, %r10, 0x0300
        STORE.W 0xFF000000, %r3
        LOAD.W %r0, 0xFF000000

        ; Print Dev ID
        ADD %r1, %r1, 6
        MOV %r2, 0x70
        CALL print_hex_w

        ; Ask Version
        OR %r3, %r10, 0x0400
        STORE.W 0xFF000000, %r3
        LOAD.W %r0, 0xFF000000

        ; Print Dev Version
        ADD %r1, %r1, 6
        MOV %r2, 0x70
        CALL print_hex_w

        ; Ask JMP1
        OR %r3, %r10, 0x1000
        STORE.W 0xFF000000, %r3
        LOAD.W %r0, 0xFF000000

        ; Print Dev Jmp 1
        ADD %r1, %r1, 6
        MOV %r2, 0x70
        CALL print_hex_w

        ; Ask JMP2
        OR %r3, %r10, 0x2000
        STORE.W 0xFF000000, %r3
        LOAD.W %r0, 0xFF000000

        ; Print Dev Jmp 2
        ADD %r1, %r1, 6
        MOV %r2, 0x70
        CALL print_hex_w

        ADD %r1, %r1, 12    ; Jumps to the next row
        RJMP loop_dev         ; Next dev to ask


loop_dev_end:

        SLEEP  ; End of program

; Prints a %r0 digit in screen 0 with textmode 0, at column %r1[0:7], row %r1[8:15]
;       attribute  %r2
; Asummes that row < 30 and column < 40
print_chr:
      PUSH %r4
      PUSH %r5
      PUSH %y

      AND %r4, %r1, 0x3F          ; Grabs column and puts in %r4
      LRS %r5, %r1, 8             ; Grabs row and puts in %r5
      MUL %r5, %r5, 0x28
      ADD %r4, %r4, %r5           ; %r4 = row*40 + column
      LLS %r4, %r4, 1             ; %r4 *= 2
      ADD %r4, %r4 ,0xFF0A0000
      STORE.B %r4, %r0            ; Write character
      ADD %r4, %r4, 1
      STORE.B %r4, %r2            ; Write Attribute

      POP %y
      POP %r5
      POP %r4

      RET

      .include "lib.inc"

; *****************************************************************************
; Strings

str01:  .DB "Nº Dev: ", 0 
str02:  .DB "S  Cl  Buil  ID    Ver   J1    J2", 0
str03:  .DB "-----------------------------------", 0
