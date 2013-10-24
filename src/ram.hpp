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

	AHandler(dword_t begin, dword_t size = 1) :  _begin(begin), _size(size)
	{ }

	virtual ~AHandler()
	{ }

    /**
     * Begin of address block
     */
    dword_t begin() const
    {
        return this->_begin;
    }

    /**
     * Size of the address block
     */
    dword_t size() const
    {
        return this->_size;
    }

    /**
     * End of the address block (not inclusive)
     */
    dword_t end() const
    {
        return _begin+_size;
    }

    /**
     * Code that is executed when the CPU try to read an address of the block
     * @param addr Address to read
     * @return byte of data to return
     */
    virtual byte_t rb(dword_t addr) = 0; 

    /**
     * Code that is exeucuted when the CPU try to write to an address of the block
     * @param addr Address to read
     * @param val Byte value that try to write
     */
    virtual void wb(dword_t addr, byte_t val) = 0; 

protected:
	
    dword_t _begin;	/// Begin of the Block of memory
	dword_t _size;	/// Size of the block of memory

};

/**
 * Represents the memmory address space of the computer
 * TODO Usea better container to make lighting fast the search of a address handler. Could be a interval tree or similar
 */
class Mem {
public:

    /**
     * Generates a memmory mapping of RAM/ROM here the ROM resides in the first 
     * addreses of the addres space and fills the ROM with some data
     * @param *rom Ptr to the data to be copied to the ROM
     * @param rom_size Size of the ROM data that must be less or equal to 64KiB. Big sizes will be ignored
     * @param ram_size Size of the RAM. By default is 128KiB
     */
	Mem (const byte_t* rom, size_t rom_size, size_t ram_size = 128*1024) : 
        ram_size(ram_size), buffer(NULL)
	{
        if (rom_size > 64*1024)
            rom_size = 64*1024;
        this->rom_size = rom_size;

        buffer =  new byte_t[64*1024 + this->ram_size];
        std::copy_n(rom, this->rom_size, buffer); // Copy ROM
	}

    ~Mem()
    {
        if (buffer != NULL)
            delete[] buffer;
    }

	/**
	 * Read  operator
     * @param addr Address
	 */
	byte_t rb(dword_t addr) const
	{
        if (addr < rom_size) { // ROM ADDRESS
            return buffer[addr];
        } else if (addr > 64*1024 && addr < (64*1024 + ram_size)) { // RAM ADDRESS
            return buffer[addr];
        } 
		
        // Search the apropiated block
		for (auto it= blocks.begin(); it != blocks.end(); ++it) {
			if ( (*it)->begin() <= addr && ((*it)->end() > addr)) {
				return (*it)->rb(addr);
			}
		}

		return 0; // Not found address, so we return 0
	}

	/**
	 * Write operator
     * @param addr Address
     * @param val Byte value to been writen
	 */
	void wb(dword_t addr, byte_t val) {
        if (addr < rom_size) { // ROM ADDRESS, we do nothing
            return;
        } else if (addr > 64*1024 && addr < (64*1024 + ram_size)) { // RAM ADDRESS
            buffer[addr] = val;
            return;
        } 
		
        // Search the apropiated block
		for (auto it= blocks.begin(); it != blocks.end(); ++it) {
			if ( (*it)->begin() <= addr	&& ((*it)->end() > addr) ){
                (*it)->wb(addr, val);
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
	bool addBlock(std::shared_ptr<AHandler> block)
	{
        // TODO Use other data structure to contains the blocks
		blocks.push_back(block);
        return true;
	}

private:
    size_t rom_size;
    size_t ram_size;

    byte_t* buffer; /// Contains the ROM + RAM as are contigous

	std::vector<std::shared_ptr<AHandler>> blocks; ///Contains address space blocks

};

} // End of namespace ram
} // End of namespace vm

#endif
