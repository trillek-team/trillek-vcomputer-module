        ;.ORG 0x8000 ; At 32 Kib ...

        .DD 0xBEBECAFE

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


; Print HEX byte function
; %r0 <- Byte to print
; %r1 <- Ptr to text screen buffer palce to print
; %r2 <- Color atributte
; return nothing
print_hex_b:
        PUSH %r5

        LLS %r2, %r2, 8
        LRS %r5, %r0, 4         ; MS nible to %r5
        AND %r0, %r0, 0x0000000F      ; LS nible to %r0

        ; Print MS nible first
        ; 0xA + 55 = 'A'
        ; 0x9 + 55 = 64 -> 64 -7 = 57 = '9'
        ADD %r5, %r5, 55
        IFL %r5, 'A'
          SUB %r5, %r5, 7

        OR %r5, %r5, %r2
        STORE.W %r1, %r5

        ; Print LS nible first
        ADD %r1, %r1, 2
        ADD %r5, %r0, 55
        IFL %r5, 'A'
          SUB %r5, %r5, 7

        OR %r5, %r5, %r2
        STORE.W %r1, %r5
        
        POP %r5

        RET


; Print HEX word function
; %r0 <- Word to print
; %r1 <- Ptr to text screen buffer palce to print
; %r2 <- Color atributte
; return nothing
print_hex_w:
        PUSH %r5

        LLS %r2, %r2, 8
        LRS %r5, %r0, 12         ; MS nible to %r5

        ; Print MS nible first
        ; 0xA + 55 = 'A'
        ; 0x9 + 55 = 64 -> 64 -7 = 57 = '9'
        ADD %r5, %r5, 55
        IFL %r5, 'A'
          SUB %r5, %r5, 7

        OR %r5, %r5, %r2
        STORE.W %r1, %r5

        ; Print next nible first
        LRS %r5, %r0, 8
        AND %r5, %r5, 0x0F

        ADD %r1, %r1, 2
        ADD %r5, %r5, 55
        IFL %r5, 'A'
          SUB %r5, %r5, 7

        OR %r5, %r5, %r2
        STORE.W %r1, %r5
        
        ; Print next nible first
        LRS %r5, %r0, 4
        AND %r5, %r5, 0x0F

        ADD %r1, %r1, 2
        ADD %r5, %r5, 55
        IFL %r5, 'A'
          SUB %r5, %r5, 7

        OR %r5, %r5, %r2
        STORE.W %r1, %r5
        
        ; Print next nible first
        AND %r5, %r0, 0x0F

        ADD %r1, %r1, 2
        ADD %r5, %r5, 55
        IFL %r5, 'A'
          SUB %r5, %r5, 7

        OR %r5, %r5, %r2
        STORE.W %r1, %r5
        
        POP %r5

        RET


; Clear screen. Fills the scren with spaces and a particular color attribute
; %r0 <- Ptr to text screen buffer
; %r1 <- Color atributte
clr_screen:
        MOV %r3, 2400 ; Ptr to the end of the screen
        ADD %r3, %r3, %r0

        LLS %r1, %r1, 8
        OR %r1, %r1, 0x20  ; Prepare fill word

clr_screen_loop:
        STORE.W %r0, %r1
        ADD %r0, %r0, 2

        IFLE %r0, %r3
          RJMP clr_screen_loop

        RET
