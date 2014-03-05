#include "VComputer.hpp"

#include "gtest/gtest.h"

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

