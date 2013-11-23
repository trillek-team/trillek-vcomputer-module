; Hello World in RC3200 ASM

        MOV %sp, 0x30000    ; Sets Stack Pointer to the end of the 128KiB RAM     
        
        MOV %r0, 0
        STORE.B 0xFF0ACC00, %r0 ; Text mode 0, default font and palette, no vsync
       
        ; Call to Print function
        MOV %r0, string         ; %r0 ptr to string
        MOV %r1, 0xFF0A0000      ; %r1 ptr to screen
        MOV %r2, 0x70           ; Light gray paper, black ink

        CALL print

        SLEEP                   ; End of program 

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

;******************************************************************************
; Data
string: .DB 'H', 'e', 'l', 'l', 'o', 0x20, 'w', 'o', 'r', 'l', 'd', '!', 0 ; ASCIIz string
