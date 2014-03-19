  ; Hello World in TR3200 ASM
  .ORG 0x100000       ; This a ROM image, so jumps need to know the real address

  MOV %sp, 0x20000    ; Sets Stack Pointer to the end of the 128KiB RAM

  ; Code to find the first TDA plugged device
  MOV %r10, 0x10FF00
begin_search_tda:
  ADD %r10, %r10, 0x100
  IFEQ %r10, 0x112100  ; Not found any TDA device
    JMP end

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
  ; Configure TDA to use a text buffer in 0x001000
  ADD %r0, %r10, 0x0A

  MOV %r1, 0x001000
  STORE %r0, %r1 ; Set B:A to point to 0x001000

  ADD %r0, %r10, 0x08
  MOV %r1, 0
  STORE.W %r0, %r1 ; Send command to point Text buffer to B:A address

  ; Clears the screen
  MOV %r0, 0x001000
  MOV %r1, 0xB0           ; Dark brown paper, Black Ink
  CALL clr_screen

  ; Prints the string
  MOV %r1, 0x001000       ; %r1 ptr to text buffer
  MOV %r0, string         ; %r0 ptr to string
  MOV %r2, 0x4F           ; Dark blue paper, White Ink
  CALL print

  ; Prints the string at Rom 3, Column 3
  MOV %r0, 3              ; Row 3
  MOV %r1, 3              ; Column 3
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; Adds the offset to the ptr to text buffer

  MOV %r0, string         ; %r0 ptr to string
  MOV %r2, 0x78           ; Swamp paper, Yellow Ink
  CALL print

end:
  SLEEP                   ; End of program


  .include "BROM.inc"


;******************************************************************************
; Const Data
string: .DB "Hello world!", 0   ; ASCIIz string

;******************************************************************************
; RAM data
  .ORG 0
  ; Nothing...
