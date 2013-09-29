RC1600 CPU prototype
====================

Specs : https://gist.github.com/Zardoz89/6240295

Actually does this:

 - Allow to select run mode or step mode
 - In run mode, shows a **MDA** 80x25 like **screen** in stdout, **mapped at B:0000h**
 - In step mode, show register status, last values of Stack, instruction being executed in human form, also executes a single instrucction every time that a key is pressed. If is pressed '**q**' or '**Q**', then ends the emulation.


TODO :
------

 - INP and OUT instruction
 - Better interface with RAM to allow to use seamless ROM and memory mapped devices, plus bound checking to not overrun
 - Do more extensive tests
 - Implement or adapt an assembler to the RC1600 ASM (imperative!)