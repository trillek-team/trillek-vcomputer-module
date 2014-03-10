Trillek Virtual Computer Lib
============================

Using these [specs](https://github.com/trillek-team/trillek-computer)

The design of the Virtual Computer should allow to plug and use different CPUs. 

This repo is composed by the Trillek Virtual Computer lib and some extra tools. 

| linux                                            |
|--------------------------------------------------|
| [![Build Status](https://travis-ci.org/trillek-team/trillek-vcomputer-module.png?branch=new-version)](https://travis-ci.org/trillek-team/trillek-vcomputer-module) |

COMPILING
---------
Actually I'm using CMake and C++ 11. 

    mkdir build
    cd build
    cmake ..
    make

By default will generate dynamic libs and compile tools and tests.

If you wish to compile a static lib, then you should use **"cmake -DBUILD_STATIC_TR3200=True .."**

If you not wish to compile the tools, then you should use **"cmake -DBUILD_TOOLS_TR3200=False .."**

If you not wish to compile the tests, then you should use **"cmake -DBUILD_TESTS_TR3200=False .."**

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

Is a program that uses Trillek Virtual Computer lib to run a TR3200 emulation. Can run in step mode or in "run mode" where executes all the program without stopping. Needs C++ 11 *std::chrono compilance* for timings; **SDL2 + GLFW + GLM + OpenGL 3.1** is needed if you desire to see the virtual screen.

- Can load a little endian binary file with a program. Has a 64KiB ROM were the program is uploaded. 128KiB of RAM begins at 0x10000 and ends at 0x30000.
- Step mode, have a on-line disassembler. Each time that you press a
  key, one instruction is executed, and the status of registers and stack, is
  shown. '**q**' ends the execution.
- Run mode, It try to enforce CPU Clock speed (100KHz) with a simple algorithm. Only stops by doing '**Ctrl+C**'. 
- If SDL2, GLM and OpenGL libs headers are found, then it will create a window of 800x600 showing a virtual screen. This
  adds the possibility of end the execution closing the window or pressing '**q**'. Additionally '**F3**' key toggles virtual keyboard. When Virtual Keyboard is enabled, 'q' key types to it instead of ending the emulation.

#### benchmark

Is a program that run a quick and dirty benchmark to measure the performance of the Trillek Virtual Computer lib. Needs C++ 11 *std::chrono compilance* for timings.

- Can load a list of little endian binary files as ROMs for each Virtual Computer. Has a 64KiB ROM were the program is uploaded. 128KiB of RAM begins at 0x10000 and ends at 0x30000.
- Executes a random number of cycles before begin the benchmark to make more realistic.
- Tries to execute 1000 Virtual Computers with the TR3200 CPUs in a single thread at what your computer can give. It does not enforce a particular speed, but it does compare the real speed against a CPU clock of 100KHz. Only stops by doing 'Ctrl+C'.

#### pbm2font

Is a small tool that can generate a hexadecimal representation of a user font for the CDA display card. Uses as source image, a b&w **pbm** file (Gimp can generate it). It expects that the image have a width and height multiple of 8, as it divides the image in cells of 8x8 pixels each being a glyph of the font.

### Tests

#### test_cda

It tests the CDA VRAM to RGBA Texture routines, PBO texture streaming, and displays it in a virtual screen in 3D (in a window of 1024x768). This requires SDL2 + GLFW + GLM + OpenGL 3.1 to work.
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

ADJUNCT TR3200 ASM PROGRAMS
--------------------------
There is some TR3200 ASM programs, source code and binary files, in /asm directory. Actually are :

- hwm.asm : List the number of attached devices and display his enumeration information
- hello.asm : Hello world
- test.asm : Some tests of TR3200 CPU compliance.
- type.asm : Basic type program
- clock.asm : Basic clock that prints hours minutes seconds in hexadecimal, and uses PIT TMR0 interrupt plus sleeps waiting it.

IMPLEMENTED DEVICES
-------------------


TODO
----

- Nearly all, as we are moving to the new version of the specs.

