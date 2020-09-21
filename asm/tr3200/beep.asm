    ; Test beep in TR3200 ASM
    .ORG 0x100000       ; This a ROM image, so jumps need to know the real address

    MOV %sp, 0x020000    ; Sets Stack Pointer to the end of the 128KiB RAM

;******************************************************************************

    MOV %r0, 932            ; BIOS like beep
    STOREW 0x11E020, %r0

    MOV %r1, 0
wait_loop0:
    ADD %r0, %r0, 0         ; NOP
    ADD %r1, %r1, 1
    IFL %r1, 8000
        RJMP wait_loop0

    MOV %r0, 0              ; Stop sound
    STOREW 0x11E020, %r0

    MOV %r1, 0
wait_loop1:
    ADD %r0, %r0, 0         ; NOP
    ADD %r1, %r1, 1
    IFL %r1, 8000
        RJMP wait_loop1

; for f=600hz to 500hz step 10hz
;   sound f
;   delay

    MOV %r0, 600
begin:
    STOREW 0x11E020, %r0

    MOV %r1, 0
wait_loop2:
    ADD %r0, %r0, 0         ; NOP
    ADD %r1, %r1, 1
    IFL %r1, 1000
        RJMP wait_loop2

    SUB %r0, %r0, 10
    IFL %r0, 500
        RJMP again
    RJMP begin

again:
    MOV %r0, 600
    RJMP begin

    SLEEP


