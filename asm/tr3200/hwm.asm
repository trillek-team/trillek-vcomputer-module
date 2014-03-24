  ; Hello World in TR3200 ASM
  .ORG 0x100000       ; This a ROM image, so jumps need to know the real address

  MOV %sp, 0x020000    ; Sets Stack Pointer to the end of the 128KiB RAM

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
  STORE.W %r0, %r1 ; Send command to point Text buffer to B:A address

  ; Clears the screen
  MOV %r0, 0x001000
  MOV %r1, 0x40           ; Dark blue paper, Black Ink
  CALL clr_screen

;******************************************************************************
  ; Get a list of pluged devices
  MOV %r0, dev_table
  CALL hwn
  STORE n_dev, %r0

;******************************************************************************
  ; Print "Nº Dev :0x"
  MOV %r0, 29             ; Row 29
  MOV %r1, 0              ; Column 0
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print

  MOV %r0, str01
  MOV %r2, 0x4F           ; Dark blue paper, Yellow Ink
  CALL print

  ; Print number of devices in hex
  MOV %r0, 29             ; Row 29
  MOV %r1, 10             ; Column 10
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print

  LOAD.B %r0, n_dev
  MOV %r2, 0x48           ; Dark blue paper, Yellow Ink
  CALL print_hex_b

  ; Print Header
  MOV %r0, 0              ; Row 0
  MOV %r1, 0              ; Column 0
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print

  MOV %r0, str02
  MOV %r2, 0x4F           ; Dark blue paper, White Ink
  CALL print

  ; Print Separator
  MOV %r0, 1              ; Row 1
  MOV %r1, 0              ; Column 0
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print

  MOV %r0, str03
  MOV %r2, 0x49           ; Dark blue paper, Copper Ink
  CALL print

;******************************************************************************
  ; Prints the list
  LOAD.B %r10, n_dev
  MOV %r5, 0  ; %r5 is the counter of the for loop

:for_loop
  LLS %r6, %r5, 3         ; %r2 points to the begin of each element of the list

 ; *** Print Slot number
  ADD %r0, %r5, 2         ; Row %r5 +2
  MOV %r1, 2              ; Column 2
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print
  MOV %r2, 0x48           ; Dark blue paper, Yellow Ink
  LOAD.B %r0, %r6         ; Read slot number
  CALL print_hex_b        ; And print it

 ; *** Print Dev Type
  ADD %r6, %r6, 1
  ADD %r0, %r5, 2         ; Row %r5 +2
  MOV %r1, 9              ; Column 9
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print
  MOV %r2, 0x48           ; Dark blue paper, Yellow Ink
  LOAD.B %r0, %r6         ; Read slot number
  CALL print_hex_b        ; And print it

 ; *** Print Dev SubType
  ADD %r6, %r6, 1
  ADD %r0, %r5, 2         ; Row %r5 +2
  MOV %r1, 16             ; Column 16
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print
  MOV %r2, 0x48           ; Dark blue paper, Yellow Ink
  LOAD.B %r0, %r6         ; Read slot number
  CALL print_hex_b        ; And print it

 ; *** Print Dev ID
  ADD %r6, %r6, 1
  ADD %r0, %r5, 2         ; Row %r5 +2
  MOV %r1, 26             ; Column 26
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print
  MOV %r2, 0x48           ; Dark blue paper, Yellow Ink
  LOAD.B %r0, %r6         ; Read slot number
  CALL print_hex_b        ; And print it

 ; *** Print Dev ID
  ADD %r6, %r6, 1
  ADD %r0, %r5, 2         ; Row %r5 +2
  MOV %r1, 30             ; Column 30
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print
  MOV %r2, 0x48           ; Dark blue paper, Yellow Ink
  LOAD %r0, %r6           ; Read slot number

  AND %r0, %r0, 0xFFFF    ; TODO Implement print_hex_dw and remove this line
  CALL print_hex_w        ; And print it

:for_loop_check
  ADD %r5, %r5, 1
  IFL %r5, %r10
    RJMP for_loop


:crash
  SLEEP


  .include "BROM.inc"

;******************************************************************************
; Const Data
str01:  .db "Nº Dev: 0x", 0
str02:  .db "| Slot | Type | SubType | ID | Vendor |", 0
str03:  .db "+------+------+---------+----+--------+", 0

;******************************************************************************
; RAM data
  .ORG 0x0
:n_dev            .db 0
:dev_table        .dw 0

  .ORG 0x0200 ; Some special variables
:TDA_base_dev     .dw 0

