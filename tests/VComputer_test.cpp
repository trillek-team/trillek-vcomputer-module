/**
 * Unit tests of VComputer
 */
#include "VComputer.hpp"

#include "gtest/gtest.h"

#include <cstdlib>
#include <cstdio>
#include <ctime>

class VComputer_test : public ::testing::Test {
	protected:
		vm::VComputer vc;
		vm::byte_t rom[1024] = {'H','e','l','l','o',' ','w','o','r','l','d','!'};
		
		virtual void SetUp() {
			vc.SetROM(this->rom, 1024);

		}

		
};

TEST_F(VComputer_test, ReadROM) {
	vm::byte_t  byte  = vc.ReadB (0x100000);
	vm::word_t  word  = vc.ReadW (0x100000);
	vm::dword_t dword = vc.ReadDW(0x100000);

	ASSERT_EQ(byte,  'H');
	ASSERT_EQ(word,  'H' | ('e' << 8) );
	ASSERT_EQ(dword, 'H' | ('e' << 8) | ('l' << 16) | ('l' << 24) );

	byte  = vc.ReadB (0x100001);
	word  = vc.ReadW (0x100001);
	dword = vc.ReadDW(0x100001);

	ASSERT_EQ(byte,  'e');
	ASSERT_EQ(word,  'e' | ('l' << 8) );
	ASSERT_EQ(dword, 'e' | ('l' << 8) | ('l' << 16) | ('o' << 24) );

	byte  = vc.ReadB (0x100008);
	word  = vc.ReadW (0x100008);
	dword = vc.ReadDW(0x100008);

	ASSERT_EQ(byte,  'r');
	ASSERT_EQ(word,  'r' | ('l' << 8) );
	ASSERT_EQ(dword, 'r' | ('l' << 8) | ('d' << 16) | ('!' << 24) );

	byte  = vc.ReadB (0x10000C);
	ASSERT_EQ(byte,  0);
	
}

TEST_F(VComputer_test, WriteROM) {
	vc.WriteB(0x100000, 'X');
	
	vm::byte_t byte = vc.ReadB (0x100000);
	ASSERT_EQ(byte,  'H');
}

TEST_F(VComputer_test, RW_RAM) {
	std::srand(std::time(0));
	for (int i=0; i< 1024 ; i++) {
		vm::dword_t addr  = std::rand() & 0x01FFFF; // Address between 0 and 128 KiB
		vm::byte_t  bval  = 0x55;
		vc.WriteB(addr, bval);
		vm::byte_t  byte  = vc.ReadB (addr);
		ASSERT_EQ (bval, byte);

		addr  = std::rand() & 0x01FFFF; // Address between 0 and 128 KiB
		vm::word_t  wval  = 0x5A5A;
		vc.WriteW(addr, wval);
		vm::word_t  word  = vc.ReadW (addr);
		ASSERT_EQ (wval, word);

		addr  = std::rand() & 0x01FFFF; // Address between 0 and 128 KiB
		vm::dword_t dwval  = 0xBEBACAFE;
		vc.WriteDW(addr, dwval);
		vm::dword_t dword  = vc.ReadDW (addr);
		ASSERT_EQ (dwval, dword);
	}
}

