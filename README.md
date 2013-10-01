RC1600 CPU prototype
====================

Specs : https://gist.github.com/Zardoz89/6240295

Actually does this:

 - Allow to select run mode or step mode
 - Have a **real ROM** of 32 Kib from 0:0000h to 0:8000h
 - In run mode, shows a **MDA** 80x25 like **screen** in stdout, **mapped at B:0000h**
 - In step mode, show register status, last values of Stack, instruction being executed in human form, also executes a single instrucction every time that a key is pressed. If is pressed '**q**' or '**Q**', then ends the emulation.

COMPILING
---------
Actually I'm using CMake, and I need SFML for graphics/events/timing

    mkdir build
    cd build
    cmake ..
    make
 
If all it's OK, should generate a executable called **vm**, and in ./src/ should be created static and shared libs of the VM core.


TODO :
------

 - INP and OUT instruction
 - Use a better container to store address blocks and make light fast retriving the apropiate block
 - Do more extensive tests
 - Implement or adapt an assembler to the RC1600 ASM (imperative!)
