Trillek Virtual Computer Lib
============================

Using these [specs](https://github.com/trillek-team/trillek-computer)

The design of the Virtual Computer allows to plug and use different CPUs. Actually is the TR3200. 

This repo contains of the Trillek Virtual Computer lib and some extra tools, including a toy emulator build over the lib. 

| GNU/Linux                                        | Windows (VStudio 2012)  |
|--------------------------------------------------|-------------------------|
| [![Build Status](https://travis-ci.org/trillek-team/trillek-vcomputer-module.png?branch=new-version)](https://travis-ci.org/trillek-team/trillek-vcomputer-module) | [![Build status](https://ci.appveyor.com/api/projects/status/25nu8anmfjea1cnb?svg=true)](https://ci.appveyor.com/project/Zardoz89/trillek-vcomputer-module) |

COMPILING
---------
We are using CMake and C++ 11 (Vs2013/14 , G++ >= 4.7, LLVM). So if you are in GNU/Linux you could compile with this :

    mkdir build
    cd build
    cmake ..
    make

By default it will generate a dynamic library and compile tools and tests.

If you wish to compile a static lib, then you should use `cmake -DBUILD_STATIC_VCOMPUTER=True ..`
** This is required to build in MSVC **

If you not wish to compile the tools, then you should use `cmake -DBUILD_TOOLS_VCOMPUTER=False ..`. Also yo ucan control if you like to have virtual screen and audio with BUILD_TOOLS_SCREEN and BUILD_TOOLS_AUDIO (By default both are true)

If you not wish to compile the tests, then you should use `cmake -DBUILD_TESTS_VCOMPUTER=False ..`

Plus you can control if you wish release or debug building using `-DCMAKE_BUILD_TYPE=Release` or `-DCMAKE_BUILD_TYPE=Debug` flag in cmake.

In windows, you should add the "-g" parameter with the apropiated generator for VS2013 (you can get the list running cmake --help), or you can use the CMake GUI. With this, you can generate VS2013/4 project/solution files that you can open and compile, or use msbuild from a develop console.

WHAT IT DOES ACTUALLY
------------------
### Tools

#### vm

Is a program that uses the Trillek Virtual Computer lib to run a TR3200 emulation. Can run in step mode or in "run mode" where it executes all the program without stopping. Needs C++ 11 *std::chrono compilance* for measuring times; **GLFW3 + GLM + OpenGL 3.2** is needed if you desire to see the virtual screen.

- Can load a little endian binary file with a ROM image with a max size of 32KiB.
- Step mode, has an on-line disassembler. Each time that you press enter or 's' or 'S'
  keys, one instruction is executed, and the status of registers and stack, is
  shown. '**q**' ends the execution. '**r**' switch to Run mode
- Run mode, It tries to enforce CPU Clock speed. Only stops by doing '**Ctrl+C**' or closing the virtual screen window, or when finds a breakpoint. 

It accepts this parameters:
```
Virtual Computer toy Emulator

Usage:
        ./vm -r romfile [other parameters]

Parameters:
        -r file or --rom file : RAW binary file for the ROM 32 KiB
        -d file or --disk file : Disk file
        -c val or --cpu val : Sets the CPU to use, from "tr3200" or "dcpu-16n"
        -m val or --disk val : How many RAM have the computer in KiB. Must be > 128 and < 1024. Will be round to a multiple of 128
        --clock val : CPU clock speed in Khz. Must be 100, 250, 500 or 1000.
        -b val : Inserts a breakpoint at address val (could be hexadecimal or decimal).
        -h or --help : Shows this help
```

#### pbm2font

Is a small tool that can generate a hexadecimal representation of a user font for the TDA display card. Uses as source image, a b&w ASCII **pbm** file (Gimp can generate it). It expects that the image have a width and height multiple of 8, as it divides the image in cells of 8x8 pixels each being a glyph of the font.

### makedisk

Is a tool that can generate floppy disk images of different sizes and could write on a disk image, raw binary data from a file.

```
makedisk

Usage:
    makedisk -o outputfile [other parameters]

Parameters:
    -o file : Output file
    -i file : RAW binary file to be included on the floppy disk image
    -s sectors : Sectors per track (8 to 36)
    -t tracks : Tracks per side (between 40 and 80)
    -1 : Creates a floppy with only a sigle side
    -160 : Creates a 160KiB disk with a single side, 8 sectors and 40 tracks.
    -320 : Creates a 320KiB disk with 8 sectors and 40 tracks. Default floppy size
    -360 : Creates a 360KiB disk with 9 sectors and 40 tracks.
    -640 : Creates a 640KiB disk with 8 sectors and 80 tracks.
    -720 : Creates a 720KiB disk with 9 sectors and 80 tracks.
    -1200 : Creates a 1200KiB disk with 15 sectors and 80 tracks.
    -h : Shows this help
```

### Tests

#### benchmark

Is a program that runs a quick and dirty benchmark to measure the performance of the Trillek Virtual Computer lib. Needs C++ 11 *std::chrono compilance* for measuring times.

- Can load a list of little endian binary files as ROMs for each Virtual Computer.
- Aceepts a final optional parameter with the number of CPUs to run, if not especified, then will be 1000.
- Each Virtual Computer have the same set of devices plugged and the same amount of RAM
- Executes a random number of cycles before begin the benchmark to make more realistic.
- Runs CPUs at different clock speeds, aroung 70% are at 100KHz, 20% at 200KHz, 5% at 500KHz and 1% at 1Mhz.
- Tries to execute Virtual Computers with the TR3200 CPUs in a single thread at what your computer can give. It does not enforce a particular speed, but it gives information of the emulation speed. To know how many Virtual Computer you can run in your computer, tune the number of CPUs to get a speed around 110%, and these value will be the number of CPUs adecuate for your machine.
- Only stops by doing 'Ctrl+C'.
- Try it with with **Release** compile mode, there is an apreciable difference.

#### unit test

If before calling CMake, you setup a enviroment variable called "GTEST_ROOT" with the path to GTest suite, then the unit tests could be compiled and will generate an executable file "unit_test". This allow to check if some basic code gets broken if someone try to edit the source code. Are far of being a exaustive tests, but it's far better that nothing.

To set a enviroment variable on GNU/Linux (and Mac):

    export GTEST_ROOT=...

If you are using fish shell :

    set -x GTEST_ROOT /usr/src/gtest/
  
In windows, you should do that in Control Panel.

HOW I CAN CREATE PROGRAMS ?
---------------------------
Actually you have two options :

- [Meisaka's WaveAsm](https://github.com/Meisaka/WaveAsm) to generate TR3200 machine code.
- [VASM](https://github.com/Zardoz89/VASM) is a portable and retargetable assembler. It have a TR3200 module that can generate flat binary files and VOBJ object files. It have support for assembly macros and other fancy stuff.

ADJUNCT TR3200 ASM PROGRAMS
--------------------------
There are some TR3200 ASM programs, source code and binary files, in /asm directory. Specifically :

- type1.asm : Basic typing program (uses busy wait)
- test.asm : Some tests of TR3200 CPU compliance and checks. Should show an OK on screen.
- random.asm : Uses the RNG generator. Only to check that it works on step-mode of "vm"
- hwn.asm : List how many devices are pluged and what are.
- hello.asm : Hello world
- floppy.asm : Writes data to a floppy. You could check that "disk.dsk" file get filled with some data.
- clock.asm : Basic example of system clock using Timer0
- beep.asm : Generates some sound with the beeper.

Also, you can download an functional firmware from :

- [Zardoz89/trillek-firmware](https://github.com/Zardoz89/trillek-firmware) : An firmware that try to boot from an floppy and if it fails, executes a simple machine code monitor (an clone of Wozniak monitor of Apple 1 and Apple ][)
- [meisys](http://eciv.net/meisaka/meisys.ffi) : An firmware that try to boot from a floppy and shows some useful information. Also, sets a system clock. Actually only have the list command to list devices attached to the machine.


IMPLEMENTED DEVICES
-------------------

- TDA graphics card
- Generic Wester/Latin keyboard
- Beeper, RTC, RNG and Timer embed devices
- Floppy drive
- Dummy device (Used only to test stuff)
- Debug Serial Console (only to be used in develop of the virtual computer and Trillek itself)

TODO
----

- [ ] Integrated stuff of the mother board (NVRAM)
- [ ] Check interrupts
- [ ] Fix Debug mode of TR3200 cpu
- [ ] DMA (There is an old PR that is broken. Need to be fix)
- [ ] Tool to create floppy disk files and put on it flat binary files from WaveAsm or VASM
