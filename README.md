TR3200 Computer Virtual Machine Lib
===================================

Using these [specs](https://github.com/Zardoz89/Trillek-Computer)

This repo is composed by the TR3200-VM libs and some extra tools. The tools includes a program that loads a binary file to a VM ROM and executes it.

COMPILING
---------
Actually I'm using CMake and C++ 11. 

    mkdir build
    cd build
    cmake ..
    make

This will only generate the static and dynamic libs of the TR3200-VM libs.

If you wish to compile the tools, then you should use **"cmake -DBUILD_tools=True .."**

If you wish to compile the tests, then you should use **"cmake -DBUILD_tests=True .."**

Plus you can control if you wish release or debug building using **"-DCMAKE_BUILD_TYPE=Release"** or **"-DCMAKE_BUILD_TYPE=Debug"** flag in cmake.


### EMSCRIPTEN

  cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Platform/Emscripten.cmake -DEMSCRIPTEN_ROOT_PATH=/usr/bin/ ..
  make
  
Include vm_wrap.js inside a HTML page or other javascript. A example webpage will be copy to build dir. Also, you can
try it in (cpu.zardoz.es)[http://cpu.zardoz.es]

NOTE : Depending how do you install Emscripten, you will need to change the -DEMSCRIPTEN_ROOT_PATH value.

WHAT IT DOES ACTUALLY
------------------
### Tools

#### vm

Is a program that uses TR3200-VM lib to run a RC3200 emulation. Can run in step mode or in "run mode" where executes all the program without stopping. Needs C++ 11 std::chrono compilance for timings; SDL2 + GLFW + GLM + OpenGL 3.1 is needed if you desire to see the virtual screen.

- Can load a little endian binary file with a program. Has a 64KiB ROM were the program is uploaded. 128KiB of RAM begins at 0x10000 and ends at 0x30000.
- Step mode working with a on-line disassembler. Each time that you press a
  key, one instruction is executed, and the status of registers and stack, is
  shown. '**q**' ends the execution.
- Run mode at what your computer can give. It does not enforce a particular speed, but it does compare the real speed against a CPU speed of 100KHz. Only stops by doing '**Ctrl+C**'. The run speed is tweaked by source code to run at 100% aprox. in my computer.
- If SDL2, GLM and OpenGL libs headers are found, then it will create a window of 800x600 showing a virtual screen. This
  adds the possibility of end the execution closing the window or pressing '**q**'. Additionally '**F3**' key toggles virtual keyboard. When Virtual Keyboard is enabled, 'q' key types to it instead of ending the emulation.

#### benchmark

Is a program that run a quick and dirty benchmark to measure the performance of the TR3200-VM lib. Needs C++ 11 std::chrono compilance for timings.

- Can load a little endian binary file with a program. Has a 64KiB ROM were the program is uploaded. 128KiB of RAM begins at 0x10000 and ends at 0x30000.
- Trys to execute 1000 TR3200 CPUs in a single thread at what your computer can give. It does not enforce a particular speed, but it does compare the real speed against a CPU speed of 100KHz. Only stops by doing 'Ctrl+C'.

#### pbm2font

Is a small tool that can generate a hexadecimal representation of a user font for the CDA display card. Uses as source image, a b&w **pbm** file (Gimp can generate it). It expects that the image have a width and height multiple of 8, as it divides the image in cells of 8x8 pixels each being a glyph of the font.

### Tests

#### test_cda

It tests the CDA VRAM to RGBA Texture routines, PBO texture streaming, and displays it in a virtual screen in 3D (in awindow of 1024x768). This requires SDL2 + GLFW + GLM + OpenGL 3.1 to work.
It has these shortcuts:

- 'n' Change to the next video/text mode
- 'wasd' orbits the camera around the virtual screen
- 'r' Zoom In
- 'f' Zoom Out

Example image of text mode :

<a href="http://img856.imageshack.us/img856/683/fp7n.png" target="_blank"><img width="300px" src="http://img856.imageshack.us/img856/683/fp7n.png"/></a>

HOW I CAN CREATE PROGRAMS ?
---------------------------
Actually you can use <a href="https://github.com/Meisaka/WaveAsm" target="_blank">Meisaka's WaveAsm</a> to generate TR3200 machine code.

ADJUNT TR3200 ASM PROGRAMS
--------------------------
There is some TR3200 ASM programs, source code and binary files, in /asm directory. Actually are :

- hwm.asm : List the number of attached devices and display his enumeration information
- hello.asm : Hello world
- test.asm : Some tests of TR3200 compliance.
- type.asm : Basic type program
- clock.asm : Basic clock that prints hours minutes seconds in hexadecimal, and uses PIT TMR0 interrupt plus sleeps waiting it.

IMPLEMENTED DEVICES
-------------------

- CDA (need verification that user palette and user font works)
- Hardware Enumerator
- PIT (Timers) 
- Virtual Keyboard. **vm** need to do better interface with SDL2 key events as actually does.


TODO
----

- Check that all instructions work as they should (tested around 66%). -> Add more cases and fix test.asm
- Improve the memory mapper as it is currently pretty inefficient if we add a lot of devices.
- Implement beeper
- Implement more devices.
- Improve the vm API and document it better -> doxygen
- Create a vm factory class
- Fix the virtual keyboard. Actually SDL2 key event weirdness make this lees straightforward that should be. This means that actually only works alphanumeric characters and a few symbols.
- Probably remove SDL2 dependency and use directly GLFW


