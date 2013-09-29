RC1600 CPU prototype
====================

Specs : https://gist.github.com/Zardoz89/6240295

Actually only executes instructions from a hard-coded program in a array. Each time that you press any key, executes one instruction.
I don't did yet a on-line disassembler, but you could see the all registers, and what instruction hex code was executed.

TODO :
------

 - INP and OUT instruction
 - BOVF and BOCF branch instructions
 - Step-Mode of the RC1600 (throw a interrupt before every instruction)
 - Better interface with RAM to allow to use seamless ROM and memory mapped devices, plus bound checking to not overrun
 - Implement some basic MDA like device to show output to a screen.
 - Do more extensive tests
 - Implement a on-line disassembler to see what instruction is executing.
 - Implement or adapt a assembler to the RC1600 ASM