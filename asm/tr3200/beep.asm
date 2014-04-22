    ; Test beep in TR3200 ASM
    .ORG 0x100000       ; This a ROM image, so jumps need to know the real address

    MOV %sp, 0x020000    ; Sets Stack Pointer to the end of the 128KiB RAM

;******************************************************************************

    MOV %r0, 440
    STOREW 0x11E020, %r0

    SLEEP


