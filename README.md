RC1600 CPU prototype
====================

Specs : https://gist.github.com/Zardoz89/6240295

Actually only executes instructions from a hard-coded program in a array. Each time that you press any key, executes one instruction, and shows what instruction is being executed, in human form.

TODO :
------

 - INP and OUT instruction
 - Step-Mode of the RC1600 (throw a interrupt before every instruction)
 - Better interface with RAM to allow to use seamless ROM and memory mapped devices, plus bound checking to not overrun
 - Implement some basic MDA like device to show output to a screen.
 - Do more extensive tests
 - Implement or adapt a assembler to the RC1600 ASM