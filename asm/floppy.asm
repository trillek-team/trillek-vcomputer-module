.ORG 0x100000       ; This a ROM image

vector_table .equ 0x10000

MOV %SP, 0x020000   ; Sets Stack Pointer to the end of the 128KiB RAM
MOV %IA, vector_table ; IA points to the vector table
MOV %FLAGS, 0x100   ; Enable interrupts

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
    JMP end ; We skips  print code

  ; Configure TDA to use a text buffer in 0x002000
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



mov %r0, int_m5fdd
mov %r1, 8  ; 2 * 4
store %r1, vector_table, %r0
;------------------------------------------------------------------------------

MOV %R5, 0x110000
ADD %R5, %R5, 0x000600

; is present?
MOV %R10, 0
LOADB %R0, %R5, 0x0
IFNEQ %R0, 0xFF
JMP crash

; read type
MOV %R10, 1
LOADB %R0, %R5, 0x01
IFNEQ %R0, 0x08
JMP crash

; read subtype
MOV %R10, 2
LOADB %R0, %R5, 0x02
IFNEQ %R0, 0x01
JMP crash

; read id
MOV %R10, 3
LOADB %R0, %R5, 0x03
IFNEQ %R0, 0x01
JMP crash

; read builder ID
MOV %R10, 4
LOAD %R0, %R5, 0x04
IFNEQ %R0, 0x1EB37E91
JMP crash

; read status code
MOV %R10, 5
LOADW %R0, %R5, 0x10
IFNEQ %R0, 0x01
JMP crash

; read error code
MOV %R10, 6
LOADW %R0, %R5, 0x12
IFNEQ %R0, 0x00
JMP crash

;--- initial state verified ---

; query media
MOV %R10, 7
MOV %R1, 0x0003
STOREW %R5, 0x8, %R1
LOADW %R0, %R5, 0x0A ;reg A
IFNEQ %R0, 0x280
JMP crash
LOADW %R0, %R5, 0x0C ;reg B
IFNEQ %R0, 0x0228
JMP crash
LOADW %R0, %R5, 0x0E ;reg C
IFNEQ %R0, 0x0809
JMP crash

; set up interrupts
MOV %R1, 0x02
STOREW %R5, 0x0A, %R1 ; reg A
MOV %R1, 0x00
STOREW %R5, 0x08, %R1 ; cmd

; generate data
mov %r0, 0
mov %r1, data
genLoop:
  storeb %r1, %r0, %r0
  add %r0, %r0, 1
  ifle %r0, 0x1ff
    jmp genLoop

; store data in first sector
MOV %R10, 8
MOV %R1, data
STOREW %R5, 0x0A, %R1 ;A = 0x1000
MOV %R1, 0x00
STOREW %R5, 0x0C, %R1 ;B = 0x0
MOV %R1, 0x0001
STOREW %R5, 0x0E, %R1 ;C = 1 -> CHS = 0:0:1
MOV %R1, 0x0002
STOREW %R5, 0x08, %R1 ;CMD = 0x2

_sleep1:
SLEEP
LOADW %R6, %R5, 0x10
IFNEQ %R6, 0x0001
JMP _sleep1

; read data back into another place in RAM
MOV %R10, 9
MOV %R1, 0x6000
STOREW %R5, 0x0A, %R1 ; A = 0x6000
MOV %R1, 0x00
STOREW %R5, 0x0C, %R1 ; B = 0x0
MOV %R1, 0x0001
STOREW %R5, 0x0E, %R1 ; C = 1 -> CHS = 0:0:1
MOV %R1, 0x0001
STOREW %R5, 0x08, %R1 ; CMD = 0x1

_sleep2:
SLEEP
LOADW %R6, %R5, 0x10
IFNEQ %R6, 0x0001
JMP _sleep2

; store data in second sector
MOV %R10, 10
MOV %R1, data
STOREW %R5, 0x0A, %R1 ;A = 0x1000
MOV %R1, 0x00
STOREW %R5, 0x0C, %R1 ;B = 0x0
MOV %R1, 0x0002
STOREW %R5, 0x0E, %R1 ;C = 1 -> CHS = 0:0:2
MOV %R1, 0x0002
STOREW %R5, 0x08, %R1

_sleep3:
SLEEP
LOADW %R6, %R5, 0x10
IFNEQ %R6, 0x0001
JMP _sleep3

push %r10

;******************************************************************************
print_state:

  ; Prints the apropiated string
  load %r1, failed
  mov %r0, string01       ; %r0 ptr to ok string
  ifeq %r1, 1
    mov %r0, string02     ; %r0 ptr to fail string
  MOV %r1, 0x002000       ; %r1 ptr to text buffer
  MOV %r0, string01       ; %r0 ptr to string
  MOV %r2, 0x45           ; Dark blue paper, Light Green Ink
  CALL print

end:
  SLEEP
  JMP end

; Something failed
crash:
    mov %r1, 1
    storew failed, %r1
    jmp print_state

; Interrupt handler
int_m5fdd:
    MOV %R9, %R0
    RFI

;------------------------------------------------------------------------------
LIB_memcmp: ;int memcmp ( const void * ptr1, const void * ptr2, size_t num );
; %R0 -> ptr1
; %R1 -> ptr2
; %R2 -> num
  PUSH %R3
  PUSH %R4
  PUSH %R5

  MOV %R4, 0
  MOV %R3, -1
  _memcmp_loop:
	ADD %R3, %R3, 1
    IFLE %R2, %R3
      RJMP _memcmp_loop_end
    LOADB %R4, %R0, %R3
    LOADB %R5, %R0, %R3
	SUB %R4, %R4, %R5
	IFEQ %R4, 0
    RJMP _memcmp_loop
  _memcmp_loop_end:
  MOV %R0, %R4

  PUSH %R5
  PUSH %R4
  PUSH %R3
RET

  .include "BROM.ainc"

;******************************************************************************
; Const Data
string01: .DB "Disk filled with data on sector 0 and 1. Check with an hex editor.", 0
string02: .DB "Fail.", 0

;******************************************************************************
; RAM data
  .ORG 0x0
:failed         .dw 0
:TDA_base_dev   .dw 0
:data  .db 0


