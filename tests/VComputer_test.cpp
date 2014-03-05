/**
 * Unit tests of VComputer
 */
#include "VComputer.hpp"

#include "gtest/gtest.h"

#include <cstdlib>
#include <cstdio>
#include <ctime>

class TestAddrListener : public vm::AddrListener {
	public:
		unsigned readCount  = 0;
		unsigned writeCount = 0;

		vm::byte_t ReadB (vm::dword_t addr) {
			readCount++;
			//std::fprintf(stderr, "\tReading Addr: 0x%06X\n", addr);
			return 0;
		}

		vm::word_t ReadW (vm::dword_t addr) {
			return this->ReadB(addr) | (this->ReadB(addr+1) << 8);  
		}

		vm::dword_t ReadDW (vm::dword_t addr) {
			return this->ReadW(addr) | (this->ReadW(addr+2) << 16);  
		}

		void WriteB (vm::dword_t addr, vm::byte_t val) {
			writeCount++;
			//std::fprintf(stderr, "\tWriting Addr: 0x%06X <- 0x%02X\n", addr, val);
		}

		void WriteW (vm::dword_t addr, vm::word_t val) {
			WriteB(addr   , val);
			WriteB(addr +1, val >> 8);
		}

		void WriteDW (vm::dword_t addr, vm::dword_t val) {
			WriteW(addr   , val);
			WriteW(addr +2, val >> 16);
		}

};

TestAddrListener g_addr;

/**
 * Used to store common data used by the tests
 */
class VComputer_test : public ::testing::Test {
	protected:
		vm::VComputer vc;
		vm::byte_t rom[1024] = {'H','e','l','l','o',' ','w','o','r','l','d','!'};
		vm::dword_t addr_id[3];
		
		virtual void SetUp() {
			vc.SetROM(this->rom, 1024);
			addr_id[0] = -1;
			addr_id[1] = -1;
			addr_id[2] = -1;

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
	ASSERT_EQ(0, byte);
	
}

TEST_F(VComputer_test, WriteROM) {
	vc.WriteB(0x100000, 'X');
	
	vm::byte_t byte = vc.ReadB (0x100000);
	ASSERT_EQ('H', byte);
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

TEST_F(VComputer_test, AddrListener_Test) {
	vm::Range r(0x110000, 0x1100FF);
	// Addition
	addr_id[0] = vc.AddAddrListener(r, &g_addr);
  ASSERT_NE(-1, addr_id[0]);

	// Test reads
	vc.ReadB(0x110000);
	ASSERT_EQ (1, g_addr.readCount);

	vc.ReadW(0x110000);
	ASSERT_EQ (3, g_addr.readCount);
	
	vc.ReadDW(0x110000);
	ASSERT_EQ (7, g_addr.readCount);

	vc.ReadB(0x1100F0);
	ASSERT_EQ (8, g_addr.readCount);

	// Test writes
	vc.WriteB( 0x110000, 1);
	ASSERT_EQ (1, g_addr.writeCount);
               
	vc.WriteW( 0x110000, 2);
	ASSERT_EQ (3, g_addr.writeCount);
	             
	vc.WriteDW(0x110000, 4);
	ASSERT_EQ (7, g_addr.writeCount);
               
	vc.WriteB( 0x1100F0, 1);
	ASSERT_EQ (8, g_addr.writeCount);

	g_addr.readCount = 0;
	g_addr.writeCount = 0;

	// Test adding more
	TestAddrListener t_addr2;
	TestAddrListener t_addr3;

	vm::Range r_fail(0x1100F0, 0x110100); // This must fails
	addr_id[1] = vc.AddAddrListener(r_fail, &t_addr2);
  ASSERT_EQ(-1, addr_id[1]);

	vm::Range r2(0x111000); // Only a byte
	addr_id[1] = vc.AddAddrListener(r2, &t_addr2);
  ASSERT_NE(-1, addr_id[1]);

	vm::Range r3(0x110200, 0x1102FF);
	addr_id[2] = vc.AddAddrListener(r3, &t_addr3);
  ASSERT_NE(-1, addr_id[2]);

	// Test reading/writing
	vc.ReadB(0x111000);
	ASSERT_EQ (0, g_addr.readCount);
	ASSERT_EQ (1, t_addr2.readCount);
	ASSERT_EQ (0, t_addr3.readCount);

	vc.ReadB(0x110200);
	ASSERT_EQ (0, g_addr.readCount);
	ASSERT_EQ (1, t_addr2.readCount);
	ASSERT_EQ (1, t_addr3.readCount);

	vc.WriteB(0x111000, 5);
	ASSERT_EQ (0, g_addr.writeCount);
	ASSERT_EQ (1, t_addr2.writeCount);
	ASSERT_EQ (0, t_addr3.writeCount);

	vc.WriteB(0x110200, 5);
	ASSERT_EQ (0, g_addr.writeCount);
	ASSERT_EQ (1, t_addr2.writeCount);
	ASSERT_EQ (1, t_addr3.writeCount);

	// Test removing AddrListeners
	ASSERT_TRUE(vc.RmAddrListener(addr_id[1]));
	ASSERT_TRUE(vc.RmAddrListener(addr_id[2]));

	// Test reading/writing
	vc.ReadB(0x111000);
	ASSERT_EQ (0, g_addr.readCount);
	ASSERT_EQ (1, t_addr2.readCount);
	ASSERT_EQ (1, t_addr3.readCount);

	vc.ReadB(0x110200);
	ASSERT_EQ (0, g_addr.readCount);
	ASSERT_EQ (1, t_addr2.readCount);
	ASSERT_EQ (1, t_addr3.readCount);

	vc.WriteB(0x111000, 5);
	ASSERT_EQ (0, g_addr.writeCount);
	ASSERT_EQ (1, t_addr2.writeCount);
	ASSERT_EQ (1, t_addr3.writeCount);

	vc.WriteB(0x110200, 5);
	ASSERT_EQ (0, g_addr.writeCount);
	ASSERT_EQ (1, t_addr2.writeCount);
	ASSERT_EQ (1, t_addr3.writeCount);

}



