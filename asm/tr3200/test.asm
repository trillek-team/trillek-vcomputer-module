  .ORG 0x100000       ; This a ROM image, so jumps need to know the real address

begin:
  MOV %r0, 1
  MOV %r1, 1
  MOV %r2, 2
  MOV %r3, 3
  MOV %r4, 4
  MOV %r5, 5
  MOV %r6, 6
  MOV %r7, 7
  MOV %r8, 8
  MOV %r9, 9
  MOV %r10, 10
  MOV %r11, 11
  MOV %bp, 0
  MOV %sp, 0x20000    ; Sets Stack Pointer to the end of the 128KiB RAM
  ;MOV %ia, vtable
  MOV %flags, 0x100    ; Enable interrupts
  MOV %r1, 0xBEBECAFE
  ; Tested seting registers and using bit literal

  MOV %r10 , %r1      ; %r10 = 0xBEBECAFE
  SWP %r7, %r9        ; %r9 = 7 ; %r7 = 9
  ;NOT %r0, %r0        ; %r0 = 0xFFFFFFFE
  XCHGB %r4           ; %r4 = 0x400
  XCHGW %r5           ; %r5 = 0x50000

  ;Test IFx operations **************************************************
test_ifx:                       ; PC = 0x00B0
  IFEQ %r1, 0         ; %r1 = 0xBEBECAFE, so skips
      JMP crash
  IFNEQ %r1, 0xBEBECAFE ; " ", so skips
      JMP crash

  IFL %r6, %r2        ; 6 < 2 = false, so skips
      JMP crash
  IFLE %r6, %r2       ; 6 < 2 = false, so skips
      JMP crash

  IFSL %r2, %r0       ; 2 < -1 = false, so skips
      JMP crash
  IFSLE %r2, %r0      ; 2 < -1 = false, so skips
      JMP crash

  IFBITS %r4, 0x404   ; (0x400 & 0x404) != 0 so execute
      JMP next
  JMP crash

next:
  IFCLEAR %r4, 0x404  ; (0x400 & 0x404) != 0 so skips
      JMP crash

  IFEQ %r1, 0         ; False, so must skip
      IFNEQ %r1, 0              ; True but is Chained and skiped
          IFEQ %r1, 0xBEBACAFE  ; True but is Chained and skiped
              JMP crash         ; This never should be executed

  ; Test ALU operations *************************************************
test_alu:                       ; PC = 0x010C
  ; Testing BOOLEAN instructions
  MOV %r7, 0x5555AAAA
  MOV %r6, 0xAAFFFF55
  NOT %r11, %r6           ; %r11 = 0x550000AA
  IFNEQ %r11, 0x550000AA
      JMP crash

  AND %r11, %r7, %r6      ; %r11 = 0x0055AA00
  IFNEQ %r11, 0x0055AA00
      JMP crash

  OR %r11, %r7, %r6       ; %r11 = 0xFFFFFFFF
  IFNEQ %r11, 0xFFFFFFFF
      JMP crash

  XOR %r11, %r7, %r6      ; %r11 = 0xFFAA55FF
  IFNEQ %r11, 0xFFAA55FF
      JMP crash

  BITC %r11, %r7, %r6     ; %r11 = 0x550000AA
  IFNEQ %r11, 0x550000AA
      JMP crash

  ; Testing Addition/Substraction instructions
  ADD %r11, %r8, %r4      ; %r11 = %r8 + %r4 = 0x408
  IFNEQ %r11, 0x408
      JMP crash

  ADD %r0, %r8, 1         ; %r0 = %r8 +1 = 9
  IFNEQ %r0, 9
      JMP crash

  SUB %r10, %r11, %r0     ; %r10 = %r11 - %r0 = 0x3FF
  IFNEQ %r10, 0x3FF
      JMP crash

  ;SUB %r22, %r24, 4       ; %r22 = %r24 -4 = 20 = 0x14
  ;IFNEQ %r22, 20
  ;    JMP crash

  ;SUB %r22, %r24, -4      ; %r22 = %r24 -(-4) = 28 = 0x1C
  ;IFNEQ %r22, 28
  ;    JMP crash

  ;RSB %r22, %r24, -4      ; %r22 = -4 - %r24 = -28 = 0xFFFFFFE4
  ;IFNEQ %r22, -28
  ;    JMP crash

  ; Testing Overflow
  MOV %r9, 0x40000000
  MOV %r10, 0x60000000
  ADD %r11, %r10, %r9    ; %r11 = 0x40000000 + 0x60000000 = 0xA0000000 (negative)
  IFCLEAR %flags, 2       ; If OF == 0 -> Jump to crash
      JMP crash
  ; TODO Overflow with substraction

  ; Testing Carry doing 64 bit addition and substraction
  MOV %r8,  0xFFFFFFFF    ; LSB op1
  MOV %r9,  0x00000001    ; MSB op1
  MOV %r10, 0x00000001    ; LSB op2
  MOV %r11, 0x00000100    ; MSB op2
  ; Result of addtion must be 0x00000102_00000000
  ADD %r6, %r8, %r10    ; Adds LSB
  ADDC %r7, %r9, %r11   ; Adds MSB
  IFNEQ %r6, 0
      JMP crash
  IFNEQ %r7, 0x102
      JMP crash

  ; Result of addtion must be 0xFFFFFF01_FFFFFFFE
;       SUB %r24, %r20, %r22    ; Subs LSB
;       SUBB %r25, %r21, %r23   ; Subs MSB
;       IFNEQ %r24, 0xFFFFFFFE
;           JMP crash
;       IFNEQ %r25, 0xFFFFFF01
;           JMP crash

  ; Testing Shift instructions
  ; %r6 = 0xAAFFFF55
  ; %r7 = 0x55FFFFAA
  MOV %r6, 0xAAFFFF55
  MOV %r7, 0x55FFFFAA
  LLS %r10, %r6, 8        ; %r10 = 0xFFFF5500
  IFNEQ %r10, 0xFFFF5500
      JMP crash

  LRS %r10, %r6, 8        ; %r10 = 0x00AAFFFF
  IFNEQ %r10, 0x00AAFFFF
      JMP crash

  ARS %r10, %r6, 8        ; %r10 = 0xFFAAFFFF
  IFNEQ %r10, 0xFFAAFFFF
      JMP crash
  ARS %r10, %r7, 8        ; %r10 = 0x0055FFFF
  IFNEQ %r10, 0x0055FFFF
      JMP crash

  ROTL %r10, %r6, 8       ; %r10 = 0xFFFF55AA
  IFNEQ %r10, 0xFFFF55AA
      JMP crash

  ROTR %r10, %r6, 8       ; %r10 = 0x55AAFFFF
  IFNEQ %r10, 0x55AAFFFF
      JMP crash

  ; Testing Multiplication/Division
  MOV %r8, 100
  MOV %r9, 4000000001
  MUL %r10, %r9, %r8     ; %y:%r10 = 400000000100
  IFNEQ %r10, 0x21DBA064
      JMP crash
  IFNEQ %y, 0x5D
      JMP crash

  DIV %r10, %r9, %r8     ; %r10 = 40000000 ; %y = 1
  IFNEQ %r10, 0x02625A00
      JMP crash
  IFNEQ %y, 1
      JMP crash


  MOV %r0, 0x001000
  MOV %r1, 0
  MOV %r3, 0xBEBACAFE

  ; Basic test of RAM W/R
  STORE %r0, %r3
  LOAD %r1, %r0
  IFNEQ %r3, %r1
      JMP crash

;******************************************************************************
  ; Check if we did this alredy
  LOAD %r0, TDA_base_dev
  IFNEQ %r0, 0
    JMP begin

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
    MOV %r10, 0xFFFFFFFF ; We put in the var TDA base address

  STORE TDA_base_dev, %r10 ; We put in the var that we don't found anything

  IFEQ %r10, 0xFFFFFFFF
    JMP begin ; We skips  print code

  ; ******* Print on screen *******
  ; Configure TDA to use a text buffer in 0x001000
  ADD %r0, %r10, 0x0A

  MOV %r1, 0x002000
  STORE %r0, %r1          ; Set B:A to point to 0x001000

  ADD %r0, %r10, 0x08
  MOV %r1, 0
  STOREW %r0, %r1        ; Send command to point Text buffer to B:A address

  ; Clears the screen
  MOV %r0, 0x002000
  MOV %r1, 0x40           ; Dark brown paper, Black Ink
  CALL clr_screen

  ; Prints the string
  MOV %r1, 0x002000       ; %r1 ptr to text buffer
  MOV %r0, string01       ; %r0 ptr to string
  MOV %r2, 0x45           ; Dark blue paper, Light Green Ink
  CALL print


  JMP begin

  ; TODO Signed Multiplication/Division
  ; TODO Check Division error flag

  ; TODO Check other instrucctions

:crash
  SLEEP


  .include "BROM.ainc"

;******************************************************************************
; Const Data
string01: .DB "OK!", 0   ; ASCIIz string

;******************************************************************************
; RAM data
  .ORG 0x0
:data
  .db 0

  .ORG 0x0100 ; Some special variables
:TDA_base_dev   .dw 0

