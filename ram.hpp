#ifndef __RAM_HPP__
#define __RAM_HPP__ 1

#include <vector>
#include <algorithm>
#include <memory>

namespace CPU {

typedef uint32_t dword_t;
typedef uint16_t word_t;
typedef uint8_t  byte_t;

/**
 * Defines a Memmory Block
 */
class MBlock {
public:
	size_t begin;	/// Begin of the Block of memory
	size_t size;	/// Size of the block of memory

	bool read_only; /// Read-Only ? (Writebale only by the owner device)

	byte_t* block;

	MBlock(size_t begin, size_t size = 1, bool read_only = false) : 
		begin(begin), size(size), read_only(read_only)
	{
		block = new byte_t[size];
	}

	~MBlock()
	{
		delete[] block;
	}
};

typedef std::shared_ptr<MBlock> MBlock_sptr;

/**
 * Represents the memmory address space of the computer
 */
class Mem {
public:
	byte_t dumb_dst;

	Mem (size_t address_space= 20)
	{
		max_address = (1<< address_space)-1;
	}

	/**
	 * Read  operator
	 */
	byte_t rb(size_t index) const
	{
		index &= max_address;
		// Search the apropiated block
		for (auto it= blocks.begin(); it != blocks.end(); ++it) {
			if ( (*it)->begin < index && ((*it)->begin + (*it)->size) > index){
				index -= (*it)->begin;
				return (*it)->block[index];
			}
		}

		return 0;
	}

	/**
	 * Write operator
	 */
	void wb(size_t index, byte_t val) {
		index &= max_address;
		// Search the apropiated block
		for (auto it= blocks.begin(); it != blocks.end(); ++it) {
			if ( (*it)->begin < index 
					&& ((*it)->begin + (*it)->size) > index 
					&& !((*it)->read_only) ){
				index -= (*it)->begin;
				(*it)->block[index] = val;
                return;
			}
		}

	}

	void addBlock(MBlock_sptr block)
	{
		blocks.push_back(block);
	}

private:
	size_t max_address; /// Bit mask of avalible addresses

	std::vector<MBlock_sptr> blocks; /// Contains address space block
	

};

} // End of namespace CPU

#endif
