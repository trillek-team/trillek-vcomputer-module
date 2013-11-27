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


        .include "lib.inc"
  

;******************************************************************************
; Data
string: .DB "Hello world!", 0   ; ASCIIz string
