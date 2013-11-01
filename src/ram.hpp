#pragma once
/**
 * RC3200 VM - ram.hpp
 * Address space mapping class
 *
 * Maps blocks of address space to RAM/ROM or devices
 */

#ifndef __RAM_HPP__
#define __RAM_HPP__ 1

#include "types.hpp"

#include <vector>
#include <algorithm>
#include <memory>
#include <iostream>

#include <cassert>


namespace vm {
namespace ram {


class Mem;

/**
 * Defines handler for a block of addresses
 */
class AHandler {
public:

AHandler(dword_t begin, dword_t size = 1) :  begin(begin), size(size)
{ }

virtual ~AHandler()
{ }

/**
 * Begin of address block
 */
dword_t Begin() const
{
    return this->begin;
}

/**
 * Size of the address block
 */
dword_t Size() const
{
    return this->size;
}

/**
 * End of the address block (not inclusive)
 */
dword_t End() const
{
    return begin + size;
}

/**
 * Code that is executed when the CPU try to read an address of the block
 * @param addr Address to read
 * @return byte of data to return
 */
virtual byte_t RB(dword_t addr) = 0; 

/**
 * Code that is exeucuted when the CPU try to write to an address of the block
 * @param addr Address to read
 * @param val Byte value that try to write
 */
virtual void WB(dword_t addr, byte_t val) = 0; 

protected:

dword_t begin;	/// Begin of the Block of memory
dword_t size;	/// Size of the block of memory

};

/**
 * Represents the memmory address space of the computer
 * TODO Usea better container to make lighting fast the search of an address handler. Could be a interval tree or similar
 */
class Mem {
public:

/**
 * Generates a memory mapping of RAM/ROM where the ROM resides in the lowest addresses
 * @param ram_size Size of the RAM. By default is 128KiB
 */
Mem (size_t ram_size = 128*1024) : 
    ram_size(ram_size), buffer(NULL) {
}

~Mem() {
    if (buffer != NULL)
        delete[] buffer;
}

/**
 * Writes the ROM data to the internal array
 * @param *rom Ptr to the data to be copied to the ROM
 * @param rom_size Size of the ROM data that must be less or equal to 64KiB. Big sizes will be ignored
 */
void WriteROM (const byte_t* rom, size_t rom_size) {
    if (rom_size > 64*1024)
        rom_size = 64*1024;
    this->rom_size = rom_size;

    buffer =  new byte_t[64*1024 + this->ram_size];
    std::copy_n(rom, this->rom_size, buffer); // Copy ROM
}

/**
 * Read operator. Reads a byte directly
 * @param addr Address
 */
inline byte_t RB(dword_t addr) const
{
    if (addr < rom_size) { // ROM ADDRESS
        return buffer[addr];
    } else if (addr >= 64*1024 && addr < (64*1024 + ram_size)) { // RAM ADDRESS
        return buffer[addr];
    } 
    
    // Search the apropiated block
    for (auto it= blocks.begin(); it != blocks.end(); ++it) {
        if ( (*it)->Begin() <= addr && ((*it)->End() > addr)) {
            return (*it)->RB(addr);
        }
    }

    return 0; // Not found address, so we return 0
}

/**
 * Read operator. Reads a word direclty
 * @param addr Address
 */
inline word_t RW(dword_t addr) const
{
#if (BYTE_ORDER != LITTLE_ENDIAN)
    if (addr < rom_size) { // ROM ADDRESS
        return buffer[addr] | (buffer[addr+1] << 8);
    } else if (addr >= 64*1024 && addr < (64*1024 + ram_size)) { // RAM ADDRESS
        return buffer[addr] | (buffer[addr+1] << 8);
    } 
#else
    if (addr < rom_size) { // ROM ADDRESS
        return *((word_t*)(buffer + addr));
    } else if (addr >= 64*1024 && addr < (64*1024 + ram_size)) { // RAM ADDRESS
        return *((word_t*)(buffer + addr));
    } 
#endif

    // Search the apropiated block
    for (auto it= blocks.begin(); it != blocks.end(); ++it) {
        if ( (*it)->Begin() <= addr && ((*it)->End() > addr)) {
            return (*it)->RB(addr) | ((*it)->RB(addr+1) << 8);
        }
    }

    return 0; // Not found address, so we return 0
}

/**
 * Read operator. Reads a dword direclty
 * @param addr Address
 */
inline dword_t RD(dword_t addr) const
{
#if (BYTE_ORDER != LITTLE_ENDIAN)
    if (addr < rom_size) { // ROM ADDRESS
        return buffer[addr] | (buffer[addr+1] << 8) | (buffer[addr+2] << 16) | (buffer[addr+3] << 24);
    } else if (addr >= 64*1024 && addr < (64*1024 + ram_size)) { // RAM ADDRESS
        return buffer[addr] | (buffer[addr+1] << 8) | (buffer[addr+2] << 16) | (buffer[addr+3] << 24);
    } 
#else

    if (addr < rom_size) { // ROM ADDRESS
        return *((dword_t*)(buffer + addr));
    } else if (addr >= 64*1024 && addr < (64*1024 + ram_size)) { // RAM ADDRESS
        return *((dword_t*)(buffer + addr));
    } 

#endif
    // Search the apropiated block
    for (auto it= blocks.begin(); it != blocks.end(); ++it) {
        if ( (*it)->Begin() <= addr && ((*it)->End() > addr)) {
            return (*it)->RB(addr) | ((*it)->RB(addr+1) << 8) | ((*it)->RB(addr+2) << 16) | ((*it)->RB(addr+3) << 24);
        }
    }

    return 0; // Not found address, so we return 0
}

/**
 * Write operator
 * @param addr Address
 * @param val Byte value to been writen
 */
inline void WB(dword_t addr, byte_t val) {
    if (addr < rom_size) { // ROM ADDRESS, we do nothing
        return;
    } else if (addr >= 64*1024 && addr < (64*1024 + ram_size)) { // RAM ADDRESS
        buffer[addr] = val;
        return;
    } 

    // Search the apropiated block
    for (auto it= blocks.begin(); it != blocks.end(); ++it) {
        if ( (*it)->Begin() <= addr	&& ((*it)->End() > addr) ){
            (*it)->WB(addr, val);
            return;
        }
    }
    
    return ; // Not found address, so we do nothing
}


/**
 * Adds a Address block handler
 * @param block Address Handler
 * @returns False if was any problem
 */
bool AddBlock(AHandler* block)
{
    // TODO Use other data structure to contains the blocks
    blocks.push_back(block);
    return true;
}

/**
 * Addss Address block handlers
 * @param iblocks Vector of Address Handlers
 * @returns False if was any problem
 */
bool AddBlock(const std::vector<AHandler*>& iblocks)
{
    // TODO Use other data structure to contains the blocks
    for (auto iblock : iblocks )
        blocks.push_back(iblock);
    return true;
}

private:
size_t rom_size;
size_t ram_size;

byte_t* buffer; /// Contains the ROM + RAM as are contigous

std::vector<AHandler*> blocks; ///Contains address space blocks

};

} // End of namespace ram
} // End of namespace vm

#endif
