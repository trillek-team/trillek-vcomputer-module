
            .ORG 0
            MOV %sp, 0x30000    ; Sets Stack Pointer to the end of the 128KiB RAM

            MOV %r0, 0
            STORE.B 0xFF0ACC00, %r0 ; Text mode 0, default font and palette, no vsync

            MOV %r0, 0xFF0A0000
            MOV %r1, 0x70
            CALL clr_screen

            ; Config interrupts (sets the Vector Table entry for TMR0)
            MOV %ia , vtable
            MOV %r0, isr
            ADD %r1, %ia, 4
            STORE %r1, %r0
            ; And enable it
            MOV %flags, 0x100

            ; Sets TMR0 reload value
            MOV %r0, 100000
            STORE 0xFF000044, %r0

            ; Enables TMR0 and enables his interrupt
            MOV %r0, 3
            STORE.B 0xFF000050, %r0

loop:       
            ; Row 15 -> 15 * 40 *2 = 0x4B0
            ; Print the clok in hexadecimal
            LOAD.B %r0, sec
            MOV %r1, 0xFF0A04DC
            MOV %r2, 0x70
            CALL print_hex_b
            
            LOAD.B %r0, min
            MOV %r1, 0xFF0A04D6
            MOV %r2, 0x70
            CALL print_hex_b
            
            LOAD.B %r0, hour
            MOV %r1, 0xFF0A04D0
            MOV %r2, 0x70
            CALL print_hex_b

            SLEEP  ; Hey we not need to reprint every time, only when we wake up!
    
            JMP loop


isr:
            ; TMR0 interrupt Increase the time counter
            LOAD.B %r0, sec
            ADD %r0, %r0, 1
            STORE.B sec, %r0
            
            IFL %r0, 60       ; Only increase minuts if s >= 60
              RFI
            MOV %r0, 0
            STORE.B sec, %r0

            LOAD.B %r0, min
            ADD %r0, %r0, 1
            STORE.B min, %r0
            
            IFL %r0, 60       ; Only increase hours if m >= 60
              RFI
            MOV %r0, 0
            STORE.B min, %r0

            LOAD.B %r0, hour
            ADD %r0, %r0, 1
            STORE.B hour, %r0
            
            IFL %r0, 24       ; hours %= 24
              RFI
            MOV %r0, 0
            STORE.B hour, %r0

            RFI



            .include "lib.inc"

            .ORG 0x10000
sec:        .DB 0
min:        .DB 0
hour:       .DW 0
vtable:     
