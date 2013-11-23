        MOV %sp, 0x30000    ; Sets Stack Pointer to the end of the 128KiB RAM     

        ; Setup screen
        MOV %r0, 0x80
        STORE.B 0xFF0ACC00, %r0 ; Text mode 0, default font and palette, no vsync

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

; *****************************************************************************
; Print function
; %r0 <- Ptr to Null terminated string (ASCIIz)
; %r1 <- Ptr to text screen buffer
; %r2 <- Color atributte
; return nothing
print:
        LLS %r2, %r2, 8

print_loop:
        LOAD.B %r3, %r0         ; Loads in %r3 a character
        IFEQ %r3, 0             ; If is NULL then ends
          RJMP print_end

        OR %r3, %r3, %r2        ; Add color attribute
        STORE.W %r1, %r3

        ADD %r1, %r1, 2         ; Increment pointers and do the loop
        ADD %r0, %r0, 1
        RJMP print_loop

print_end:
        
        RET

str01:  .DB 'N', 'º', 0x20, 'D', 'e', 'v', ':', 0x20, 0 
