
.org 0x0000 ; Start of ROM

; device info table
devtablelen .equ 0x2000
devtabledat .equ 0x2002
timehs .equ 0x2400
timem .equ 0x2402

set i,0
set j,0
set [devtablelen],j
set j, devtabledat
scanloop:  ; read through all devices
hwr i,a
and a,0xff
ife a,0xff     ; when we find one
jsr checkdev   ; check it
swp i
add i,1        ; next index
ifg i,31       ; end of search?
set pc,endscan ; continue
swp i
set pc,scanloop

checkdev:
set [j], i  ; save the index of the device
add j,2     ; because... just because...
add [devtablelen],1
set c,i ; get info of device type
add c,2
hwr c,x
add c,2
hwr c,y
add c,2
hwr c,z
and x,0xff00
ife x,0x0100 ; ID 0x01 (TDA)
ife y,0x8B36 ; Nya
ife z,0x1c6c ; Nya
set pc,monitor
set pc,pop

monitor: ; got a monitor to setup
swp c 
and c,0xf0
bor c,0x1106 ; IO section to page 0x6
mmw c        ; map memory page
set c,i
and c,0x0f00
bor c,0x6000
; at this point C holds the base address
; of the monitor device (TDA) IO mapped in page 0x6
; setup the monitor
set [c+0xA],0xA000
set [c+0xC],0
set [c+0x8],0 ; map the TDA to 0x00:A000 in memory
set pc,pop    ; we should be good now

endscan: ; done scanning hardware
set a, 0x7D00
set i, 0
set j, 0xA000
clearloop:    ; wipe the screen
sti [j], a    ; remember: STI is word based (goes up by two octets)
ifl i, 2400
set pc, clearloop
set i, histring
set j, 0xA000
add j, 1226   ; about the middle of buffer
set a,0x4E4E
disploop:
set b,[i]
ife b,0
set pc,timerstart  ; done at end of string
byt.l
bor b,a
sti [j],b
set pc, disploop

timerstart:
mmw 0x11EE
set [0xE004], 0xC350  ; half second
byt.h
set [0xE010], 3
ias timerisr
timerloop:
slp
set pc,timerloop

timerisr:
add [timehs], 1
ifg [timehs], 18
add [timem], 1
set c, 0x3E00
set i, 0xA25A
set a, [timehs]
shr a, 1
ifn ex, 0
set [i+2],'.'+0x3E00
ife ex, 0
set [i+2],' '+0x3E00
set x, '0'
cout:
set b,a
mod b,10
set y,x
add y,b
bor y,c
set [i], y
sub i,2
dvi a,10
ifn a,0
set pc, cout
rfi

histring:
.dw 'H','e','l','l','o',' ','W','o','r','l','d','!',0
