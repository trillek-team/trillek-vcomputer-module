  ; Simple Clock program in TR3200 ASM
  .ORG 0x100000       ; This a ROM image, so jumps need to know the real address

  MOV %sp, 0x020000   ; Sets Stack Pointer to the end of the 128KiB RAM

;******************************************************************************
  ; Code to find the first TDA plugged device
  MOV %r10, 0x10FF00
begin_search_tda:
  ADD %r10, %r10, 0x100
  IFEQ %r10, 0x112100  ; Not found any TDA device
    JMP end_search_tda

  LOAD.B %r0, %r10
  IFNEQ %r0, 0xFF   ; Device Present ?
    JMP begin_search_tda

  ADD %r1, %r10, 1
  LOAD.B %r0, %r1
  IFNEQ %r0, 0x0E   ; Is a Graphics device ?
    JMP begin_search_tda

  ADD %r1, %r10, 2
  LOAD.B %r0, %r1
  IFNEQ %r0, 0x01   ; Is TDA compatible ?
    JMP begin_search_tda

end_search_tda:
  IFEQ %r10, 0x112100
    JMP crash             ; Not found, so type on what ?
  STORE TDA_base_dev, %r10 ; We put in the var that we don't found anything

  IFEQ %r10, 0xFFFFFFFF
    JMP crash             ; We skips  print code

  ; Configure TDA to use a text buffer in 0x001000
  ADD %r0, %r10, 0x0A

  MOV %r1, 0x001000
  STORE %r0, %r1          ; Set B:A to point to 0x001000

  ADD %r0, %r10, 0x08
  MOV %r1, 0
  STORE.W %r0, %r1        ; Send command to point Text buffer to B:A address

  ; Clears the screen
  MOV %r0, 0x001000
  MOV %r1, 0x48           ; Dark blue paper, Yellow Ink
  CALL clr_screen

;******************************************************************************

  ; Config interrupts (sets the Vector Table entry for TMR0)
  MOV %ia, vtable         ; IA points to the vector table
  MOV %r0, tmr0_isr
  MOV %r1, 0x01           ; Interrupt message LSB (0x0001 for TMR0)
  LLS %r1, %r1, 2         ; x4 as each entry needs four bytes

  ADD %r1, %r1, %ia       ; vtable[%r1] (STORE %ia + %r1, %r0 )
  STORE %r1, %r0          ; vtable[%r1] = tmr0_isr function address

  MOV %flags, 0x100       ; And enable interrupts

  ; Sets TMR0 reload value
  MOV %r0, 100000
  STORE 0x11E004, %r0

  ; Enables TMR0 and enables his interrupt
  MOV %r0, 3
  STORE.B 0x11E010, %r0
  ; With 100000 reload value, the TMR0 will undeflow every second, because
  ; 100000 ticks at 100KHz of device clock, it's 1 second.

  ; Pre print the ':'s
  ; Calc offset of were we desired to rpint
  MOV %r0, 14               ; Row 14
  MOV %r1, 18               ; Column 18
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000    ; %r1 points were we disred to write
  MOV %r0, str_dots
  MOV %r2, 0x4F             ; Dark blue paper, White ink
  CALL print

;******************************************************************************
loop:
  ; Print the clok in hexadecimal

  ; *** Secs
  ; Calc offset of were we desired to rpint
  MOV %r0, 14               ; Row 14
  MOV %r1, 22               ; Column 22
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000    ; %r1 points were we disred to write

  ; Print secs
  LOAD.B %r0, sec
  MOV %r2, 0x48             ; Dark blue paper, Yellow ink
  CALL print_hex_b

  ; *** Minutes
  ; Calc offset of were we desired to rpint
  MOV %r0, 14               ; Row 14
  MOV %r1, 19               ; Column 19
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000    ; %r1 points were we disred to write

  LOAD.B %r0, min
  MOV %r2, 0x48             ; Dark blue paper, Yellow ink
  CALL print_hex_b

  ; *** Hours
  ; Calc offset of were we desired to rpint
  MOV %r0, 14               ; Row 14
  MOV %r1, 16               ; Column 16
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000    ; %r1 points were we disred to write

  LOAD.B %r0, hour
  MOV %r2, 0x48             ; Dark blue paper, Yellow ink
  CALL print_hex_b

  SLEEP  ; Hey we not need to reprint every time, only when we wake up!

  JMP loop

;******************************************************************************
tmr0_isr:
  ; TMR0 interrupt Increase the time counter
  LOAD.B %r0, sec
  ADD %r0, %r0, 1
  STORE.B sec, %r0

  IFL %r0, 60       ; Only increase min if sec >= 60, so we end the rutine
    RFI

  MOV %r0, 0        ; sec >= 60 -> sec = 0; min++;
  STORE.B sec, %r0

  LOAD.B %r0, min
  ADD %r0, %r0, 1
  STORE.B min, %r0

  IFL %r0, 60       ; Only increase hour if min >= 60, so we end the rutine
    RFI

  MOV %r0, 0        ; min >= 60 -> min = 0; hour++;
  STORE.B min, %r0

  LOAD.B %r0, hour
  ADD %r0, %r0, 1
  STORE.B hour, %r0

  IFL %r0, 24       ;If hour >=  24 then hour = 0
    RFI

  MOV %r0, 0
  STORE.B hour, %r0

  RFI

:crash
  SLEEP
  JMP crash

  .include "BROM.inc"

;******************************************************************************
; Const data
str_dots:         .DB ":  :",0  ;ASCIIZ string

;******************************************************************************
; RAM data

  .ORG 0x0
vtable:           .DB 0   ; Interrupt vector table

  .ORG 0x400              ; Our variables

sec:              .DB 0
min:              .DB 0
hour:             .DW 0

:TDA_base_dev     .DW 0

