  ; Naive Type program in TR3200 ASM
  .ORG 0x100000       ; This a ROM image, so jumps need to know the real address

  MOV %sp, 0x020000    ; Sets Stack Pointer to the end of the 128KiB RAM

;******************************************************************************
  ; Code to find the first TDA plugged device
  MOV %r10, 0x10FF00
begin_search_tda:
  ADD %r10, %r10, 0x100
  IFEQ %r10, 0x112100  ; Not found any TDA device
    JMP end_search_tda

  LOADB %r0, %r10
  IFNEQ %r0, 0xFF   ; Device Present ?
    JMP begin_search_tda

  ADD %r1, %r10, 1
  LOADB %r0, %r1
  IFNEQ %r0, 0x0E   ; Is a Graphics device ?
    JMP begin_search_tda

  ADD %r1, %r10, 2
  LOADB %r0, %r1
  IFNEQ %r0, 0x01   ; Is TDA compatible ?
    JMP begin_search_tda

end_search_tda:
  IFEQ %r10, 0x112100
    JMP crash       ; Not found, so type on what ?
  STORE TDA_base_dev, %r10 ; We put in the var that we don't found anything

  IFEQ %r10, 0xFFFFFFFF
    JMP crash ; We skips  print code

  ; Configure TDA to use a text buffer in 0x001000
  ADD %r0, %r10, 0x0A

  MOV %r1, 0x001000
  STORE %r0, %r1 ; Set B:A to point to 0x001000

  ADD %r0, %r10, 0x08
  MOV %r1, 0
  STOREW %r0, %r1 ; Send command to point Text buffer to B:A address

  ; Clears the screen
  MOV %r0, 0x001000
  MOV %r1, 0x40           ; Dark blue paper, Black Ink
  CALL clr_screen

;******************************************************************************
  ; Code to find the first Keyboard plugged device
  MOV %r10, 0x10FF00
begin_search_gkeyb:
  ADD %r10, %r10, 0x100
  IFEQ %r10, 0x112100  ; Not found any G. Keyb device
    JMP end_search_gkeyb

  LOADB %r0, %r10
  IFNEQ %r0, 0xFF   ; Device Present ?
    JMP begin_search_gkeyb

  ADD %r1, %r10, 1
  LOADB %r0, %r1
  IFNEQ %r0, 0x03    ; Is a HID device ?
    JMP begin_search_gkeyb

  ADD %r1, %r10, 2
  LOADB %r0, %r1
  IFNEQ %r0, 0x01   ; Is Western/Latin Keyboard compatible ?
    JMP begin_search_gkeyb

end_search_gkeyb:
  IFEQ %r10, 0x112100
    JMP not_found       ; Not found, so type on what ?

  STORE GKeyb_base_dev, %r10 ; We put in the var that we don't found anything

  ; Sets initial values of row and col
  MOV %r0, 0
  STOREB col, %r0
  STOREB row, %r0

;******************************************************************************
:loop
  ; Get address to the write cursor
  LOADB %r1, col
  LOADB %r0, row
  CALL get_offset_from_row_col
  ADD %r0, %r0, 0x001000

  ; Makes the cursor to blink
  LOADW %r3, blink
  ADD %r3, %r3, 1
  IFL %r3, 256
    JMP  blink_off

:blink_on
  IFL %r3, 512
    JMP blink_on_real
  MOV %r3, 0        ; Resets the counter

:blink_on_real
  STOREW blink, %r3; Increment value in blink variable

  ADD %r8, %r0, 1   ; We only need to change colors
  MOV %r3, 0xF4    ; White paper, Dark blue Ink
  STOREB %r8, %r3

  JMP read_keyb

:blink_off
  STOREW blink, %r3; Increment value in blink variable

  ADD %r8, %r0, 1   ; We only need to change colors
  MOV %r3, 0x4F     ; Dark blue paper, White Ink
  STOREB %r8, %r3


;******************************************************************************
:read_keyb
  MOV %r8, %r0        ; Put cursor ptr to r8
  LOAD %r0, GKeyb_base_dev
  ADD %r1, %r0, 0x12  ; %r1 points to E register of keyboard

  LOADW %r2, %r1     ; %r2 contains the number of key events in the keyboard
  IFEQ %r2, 0
    JMP loop          ; Nothing to read

  ADD %r1, %r0, 0x08  ; %r1 points to CMD register of keyboard
  MOV %r2, 1          ; PULL-KEY command
  STOREW %r1, %r2

  ; Now A register contains the key
  ADD %r1, %r0, 0x0A  ; %r1 points to A register of keyboard
  LOADB %r4, %r1
  ; Check the keys
  IFEQ %r4, 0x0D      ; Return KEY
    JMP enter

  IFEQ %r4, 0x08      ; Backspace KEY
    JMP delete

  IFEQ %r4, 0x14      ; Left Arrow KEY
    JMP arrow_left

  IFEQ %r4, 0x15      ; Right Arrow KEY
    JMP arrow_right

  IFL %r4, 0x20       ; Ignores some no printable characters
    JMP loop

  OR %r4, %r4, 0x4F00 ; We add attribute colors
  STOREW %r8, %r4    ; And we write it to the screen

  CALL inc_cursor

  JMP loop

;******************************************************************************
:delete
  MOV %r4, 0x4F20     ; Replace by a space
  STOREW %r8, %r4    ; And we write it to the screen
  CALL dec_cursor

  JMP loop

;******************************************************************************
:enter
  MOV %r4, 0x4F20     ; We enforce to clear actual cursor position
  STOREW %r8, %r4
  CALL next_line
  JMP loop

;******************************************************************************
:arrow_left
  ; We must clear the atribute color
  ADD %r8, %r8, 1
  MOV %r4, 0x4F
  STOREB %r8, %r4
  CALL dec_cursor
  JMP loop

;******************************************************************************
:arrow_right
  ; We must clear the atribute color
  ADD %r8, %r8, 1
  MOV %r4, 0x4F
  STOREB %r8, %r4
  CALL inc_cursor
  JMP loop

;******************************************************************************
:inc_cursor  ; Increase cursor
  LOADB %r0, col
  ADD %r0, %r0, 1
  IFL %r0, 40
    JMP inc_cursor_end

  MOV %r0, 0      ; Resets column
  LOAD %r1, row   ; Increments row
  ADD %r1, %r1, 1
  IFEQ %r1, 30
    MOV %r1, 0    ; Resets row
  STOREB row, %r1

:inc_cursor_end
  STOREB col, %r0

  RET

;******************************************************************************
:next_line
  LOAD %r1, row   ; Increments row
  ADD %r1, %r1, 1
  IFEQ %r1, 30
    MOV %r1, 0    ; Resets row
  STOREB row, %r1

  MOV %r0, 0      ; Resets col
  STOREB col, %r0

  RET

;******************************************************************************
:dec_cursor  ; Decrease cursor
  LOADB %r0, col
  SUB %r0, %r0, 1
  IFSL %r0, 0
    JMP dec_cursor_reset_col
  JMP dec_cursor_end

:dec_cursor_reset_col
  MOV %r0, 39       ; Resets column
  LOAD %r1, row     ; Increments row
  SUB %r1, %r1, 1
  IFSL %r1, 0
    MOV %r1, 29     ; Resets row
  STOREB row, %r1

:dec_cursor_end
  STOREB col, %r0

  RET

;******************************************************************************
:not_found
  ; Prints "Not found keyboard"
  MOV %r1, 0x001000       ; %r1 ptr to text buffer
  MOV %r0, str_not_found  ; %r0 ptr to string
  MOV %r2, 0x4D           ; Dark blue paper, Red Ink
  CALL print

:crash
  SLEEP

  .include "BROM.ainc"

;******************************************************************************
; Const Data
str_not_found:
  .DB "Generic Keyboard not found.", 0   ; ASCIIz string

;******************************************************************************
; RAM data
  .ORG 0x0

:col .db 0
:row .db 0
:blink .dw 0

  .ORG 0x0100 ; Some special variables
:TDA_base_dev     .dd 0
:GKeyb_base_dev   .dd 0

