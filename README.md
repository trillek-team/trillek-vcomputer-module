RC3200 Computer Virtual Machine
===============================

Using this [specs](https://github.com/Zardoz89/Trillek-Computer)

COMPILING
---------
Actually I'm using CMake, and I need SFML for graphics/events/timing

    mkdir build
    cd build
    cmake ..
    make
 
If all it's OK, should generate a executable called **vm**, and in ./src/ should be created static and shared libs of the VM core.

WHAT DOES ACTUALLY
------------------

- Can load a little endian binary file with a program, as BIOS. 64KiB of RAM
  begins at 0x10000
- Debugin mode working with a on-line dissambler. Each time that you presss a
  key, one isntruction is executed, and the status of registers and stack, is
  showed. 'q' ends the execution.
- Run mode (but not shows anything yet)

### TODO

- Check that all instructions works as should be.
- Check that software interrupts works as should be.
- Redo the memmory mapper as now is prety inefiecient.
- Implment devices and check hardware interrupts
- Encapsultae the VM in a class with a documented API, so can be linked by any
  software that will use it. This class should allow to construct the base
  computer and add devices.

