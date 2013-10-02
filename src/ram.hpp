#ifndef __RAM_HPP__
#define __RAM_HPP__ 1

#include "types.hpp"

#include <vector>
#include <algorithm>
#include <memory>
#include <iostream>

#include <cassert>

namespace CPU {


class Mem;

/**
 * Defines a Memmory Block
 */
class ABlock {
public:

	ABlock(dword_t begin, dword_t size = 1, bool read_only = false) : 
		block(NULL), _begin(begin), _size(size), read_only(read_only),
        allocated(false)
	{ }

	~ABlock()
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
     * Is real-only ? Aka writable only by owner
     */
    bool isReadOnly() const
    {
        return read_only;
    }

    /**
     * Have the block being allocted to host ram ?
     */
    bool isAllocated() const
    {
        return allocated;
    }

    byte_t* getPtr()
    {
        return block;
    }

    friend class Mem;
protected:
	byte_t* block;  /// Ptr to the allocated block
	
    dword_t _begin;	/// Begin of the Block of memory
	dword_t _size;	/// Size of the block of memory

	bool read_only; /// Read-Only ? (Writebale only by the owner device)
    bool allocated; /// Is allocated ?

};

typedef std::weak_ptr<ABlock> ABlock_wptr;

/**
 * Represents the memmory address space of the computer
 * Uses an arena to keep all address blocks in a contigous chunk of RAM of the
 * host
 * TODO Usea better container to make lighting fast the search of the address
 */
class Mem {
public:
	byte_t dumb_dst;

	Mem (size_t address_space= 20) : ram_buffer(NULL)
	{
		max_address = (1<< address_space)-1;
	}

    ~Mem()
    {
        if (ram_buffer != NULL)
            delete[] ram_buffer;
    }

	/**
	 * Read  operator
     * @param addr Address
	 */
	byte_t rb(dword_t addr) const
	{
		addr &= max_address;
		// Search the apropiated block
		for (auto it= blocks.begin(); it != blocks.end(); ++it) {
			if ( (*it)->_begin <= addr && ((*it)->end() > addr)) {
				addr -= (*it)->_begin;
				return (*it)->block[addr];
			}
		}

		return 0;
	}

	/**
	 * Write operator
     * @param addr Address
     * @param val Byte value to been writen
	 */
	void wb(dword_t addr, byte_t val) {
		addr &= max_address;
		// Search the apropiated block
		for (auto it= blocks.begin(); it != blocks.end(); ++it) {
			if ( (*it)->_begin <= addr 
					&& ((*it)->end() > addr) 
					&& !((*it)->read_only) ){
				addr -= (*it)->_begin;
				
                assert((*it)->isAllocated() );

                (*it)->block[addr] = val;
                return;
			}
		}
        return;
	}

    /**
     * Adds a Address block
     * @param begin Begin address
     * @param size Size of the block (>= 1)
     * @param ro Read Only ? (false)
     * @return a Weak ptr to a ABlock
     */
	ABlock_wptr addBlock(dword_t begin, dword_t size = 1, bool ro = false)
	{
        assert(size >= 1);
        if ((begin+size) > max_address )
            return ABlock_wptr(); // Like returning null ptr

        auto ab = std::make_shared<ABlock>(begin, size, ro);
		blocks.push_back(ab);
        return ABlock_wptr(ab);
	}

    /**
     * Allocated enought contigous memory for all memmory blocks
     * @return allocated arena size
     */
    size_t allocateBlocks()
    {
        // calc size
        size_t size = 0;
		for (auto it= blocks.begin(); it != blocks.end(); ++it) {
            size = size > (*it)->end() ? size : (*it)->end();
        }
        ram_buffer = new byte_t[size];
        std::fill_n(ram_buffer, size, 0); // Clean it

		for (auto it= blocks.begin(); it != blocks.end(); ++it) {
            (*it)->block = ram_buffer + (*it)->_begin;
            (*it)->allocated = true;
        }

        return size;
    }

private:
	size_t max_address; /// Bit mask of avalible addresses
    byte_t* ram_buffer;

	std::vector<std::shared_ptr<ABlock>> blocks; ///Contains address space blocks
	

};

} // End of namespace CPU

#endif
