RC3200 Computer Virtual Machine
===============================

Using this [specs](https://github.com/Zardoz89/Trillek-Computer)

COMPILING
---------
Actually I'm using CMake and C++ 11. Needs std::chrono compilance for timings. SDL2 + OpenGL 3.1 is need if you desire to see the virtual screen.

    mkdir build
    cd build
    cmake ..
    make
 
If all it's OK, should generate executables called **vm** and **benchmark**, and in ./src/ should be created static and shared libs of the VM core.

WHAT DOES ACTUALLY
------------------

### vm

- Can load a little endian binary file with a program. Have a 64KiB ROM were the program is uploaded. 128KiB of RAM begins at 0x10000 and ends at 0x30000.
- Step mode working with a on-line disassembler. Each time that you press a
  key, one instruction is executed, and the status of registers and stack, is
  showed. 'q' ends the execution.
- Run mode at what you computer can give. Not enforces a particular speed, but compares the real speed against the CPU speed @ 100KHz. Only stops doing 'Ctrl+C'.
- If SDL2, GLM and OpenGL libs headers are found, then will create a windows of 800x600 showing a virtual screen.

### benchmark

- Can load a little endian binary file with a program. Have a 64KiB ROM were the program is uploaded. 128KiB of RAM begins at 0x10000 and ends at 0x30000.
- Try to execute 1000 RC3200 cpus in a single thread, at what you computer can give. Not enforces a particular speed, but compares the real speed against the CPU speed @ 100KHz. Only stops doing 'Ctrl+C'.

TESTS
-----

If instrad of cmake, is executed "cmake -DBUILD_tests=True .." Some test programs will be compiled.

### test_cda

It tests the CDA VRAM to RGBA Texture routines, PBO texture streaming, and display it in a virtual screen in 3d. Requires OpenGL 3.1 to work.
Have this shortcuts:

- 'n' Change to the next video/text mode
- 'wasd' orbits the camera around the virtual screen
- 'r' Zoom In
- 'f' Zoom Out

HOW I CAN CREATE PROGRAMS ?
---------------------------
Actually you can use <a href="https://github.com/Meisaka/WaveAsm" target="_blank">Misaka's WaveAsm</a> to generate RC3200 machine code.

IMPLEMENTED DEVICES
-------------------

- CDA (need verification)
- Hardware Enumerator (need verification)


TODO
----

- Check that all instructions works as should be. -> Add more cases to test.asm
- Check that software interrupts works as should be.
- Improve the memory mapper as now is pretty inefficient if we add a lot of devices.
- Implement devices and check hardware interrupts. Actually implemented CDA device but not full checked.
- Improve the vm API and a documente it better
- Create a vm factory class

