  ; Devices connected list in TR3200 ASM
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
  ; Get a list of pluged devices
  MOV %r0, dev_table
  CALL hwn
  STOREB n_dev, %r0

;******************************************************************************
  ; Print "Nº Dev: "
  MOV %r0, 29             ; Row 29
  MOV %r1, 0              ; Column 0
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print

  MOV %r0, str01
  MOV %r2, 0x4F           ; Dark blue paper, Yellow Ink
  CALL print

  ; Print number of devices in hex
  MOV %r0, 29             ; Row 29
  MOV %r1, 8              ; Column 8
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print

  LOADB %r0, n_dev
  MOV %r2, 0x48           ; Dark blue paper, Yellow Ink
  CALL print_uint

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
  LOADB %r10, n_dev
  MOV %r5, 0  ; %r5 is the counter of the for loop

:for_loop

  ; *** Print Slot number
  ADD %r0, %r5, 2         ; Row %r5 +2
  MOV %r1, 3              ; Column 3
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print
  MOV %r2, 0x48           ; Dark blue paper, Yellow Ink

  ADD %r6, %r5, dev_table
  MOV %r0, 0
  LOADB %r0, %r6         ; Read slot number (%r0 = dev_table[%r5])
  CALL print_uint        ; And print it

  LLS %r7, %r0, 8         ; 0xXX00
  ADD %r7, %r7, 0x110000  ; 0x11XX00

  ; *** Print Dev Type
  ADD %r6, %r7, 1         ; %r6 Points to Slot XX Dev Type
  ADD %r0, %r5, 2         ; Row %r5 +2
  MOV %r1, 10             ; Column 10
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print
  MOV %r2, 0x48           ; Dark blue paper, Yellow Ink
  LOADB %r0, %r6         ; Read slot number
  CALL print_hex_b        ; And print it

  ; *** Print Dev SubType
  ADD %r6, %r7, 2         ; %r6 Points to Slot XX Dev SubType
  ADD %r0, %r5, 2         ; Row %r5 +2
  MOV %r1, 18             ; Column 18
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print
  MOV %r2, 0x48           ; Dark blue paper, Yellow Ink
  LOADB %r0, %r6         ; Read slot number
  CALL print_hex_b        ; And print it

  ; *** Print Dev ID
  ADD %r6, %r7, 3         ; %r6 Points to Slot XX Dev ID
  ADD %r0, %r5, 2         ; Row %r5 +2
  MOV %r1, 26             ; Column 26
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print
  MOV %r2, 0x48           ; Dark blue paper, Yellow Ink
  LOADB %r0, %r6         ; Read slot number
  CALL print_hex_b        ; And print it

  ; *** Print Dev ID
  ADD %r6, %r7, 4         ; %r6 Points to Slot XX Dev ID
  ADD %r0, %r5, 2         ; Row %r5 +2
  MOV %r1, 30             ; Column 30
  CALL get_offset_from_row_col
  ADD %r1, %r0, 0x001000  ; %r1 points were we like to print
  MOV %r2, 0x48           ; Dark blue paper, Yellow Ink
  LOAD %r0, %r6           ; Read slot number

  CALL print_hex_dw       ; And print it

:for_loop_check
  ADD %r5, %r5, 1
  IFL %r5, %r10
    RJMP for_loop


:crash
  SLEEP


  .include "BROM.ainc"

;******************************************************************************
; Const Data
str01:  .db "Nº Dev: ", 0
str02:  .db "| Slot | Type | SubType | ID | Vendor |", 0
str03:  .db "+------+------+---------+----+--------+", 0

;******************************************************************************
; RAM data
  .ORG 0x0
:n_dev            .db 0
:dev_table        .db 0, 1, 2, 3 ,4 ,5 ,6 ,7, 8, 9 ,10, 'a', 'b', 0, 1, 2 ,3 ,4 ,5 ,6, 7, 8

  .ORG 0x0200 ; Some special variables
:TDA_base_dev     .dw 0

