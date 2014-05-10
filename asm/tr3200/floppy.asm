.ORG 0x100000       ; This a ROM image

MOV %SP, 0x020000   ; Sets Stack Pointer to the end of the 128KiB RAM
MOV %IA, 0x10     ; IA points to the vector table
MOV %FLAGS, 0x100   ; Enable interrupts

MOV %R0, int_m5fdd
STORE 0x18, %R0
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
STOREW %R5, 0x0A, %R1
MOV %R1, 0x00
STOREW %R5, 0x08, %R1

; generate data
MOV %R0, 0
MOV %R1, 0x1000
genLoop:
  STOREB %R1, %R0, %R0
  ADD %R0, %R0, 1
  IFLE %R0, 0xFF
  JMP genLoop

; store data in first sector
MOV %R10, 8
MOV %R1, 0x1000
STOREW %R5, 0x0A, %R1 ;A = 0x1000
MOV %R1, 0x00
STOREW %R5, 0x0C, %R1 ;B = 0x0
STOREW %R5, 0x0E, %R1 ;C = 0x0
MOV %R1, 0x0002
STOREW %R5, 0x08, %R1 ;CMD = 0x2

_sleep1:
SLEEP
LOADW %R6, %R5, 0x10
IFNEQ %R6, 0x0001
JMP _sleep1

; read data back into another place in RAM
MOV %R10, 9
MOV %R1, 0x2000
STOREW %R5, 0x0A, %R1
MOV %R1, 0x0001
STOREW %R5, 0x08, %R1

_sleep2:
SLEEP
LOADW %R6, %R5, 0x10
IFNEQ %R6, 0x0001
JMP _sleep2

; store data in second sector
MOV %R10, 10
MOV %R1, 0x2000
STOREW %R5, 0x0A, %R1
MOV %R1, 0x01
STOREW %R5, 0x0E, %R1
MOV %R1, 0x0002
STOREW %R5, 0x08, %R1

crash:
  SLEEP
  JMP crash
  
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