#pragma once
/**
 * TR3200 VM - RC3200 opcodes for RC3200 v7
 * CPU of the virtual machine
 */

#ifndef __TR3200_OPCODES__
#define __TR3200_OPCODES__ 1

namespace vm {
	namespace cpu {

		// 3 Parameters OpCodes *******************************************************

		/**
		 * 3 parameters OpCodes 
		 */
		enum P3_OPCODE {
			AND             = 0x00,
			OR              = 0x01,
			XOR             = 0x02,
			BITC            = 0x03,

			ADD             = 0x04,
			ADDC            = 0x05,
			SUB             = 0x06,
			SUBB            = 0x07,
			RSB             = 0x08,
			RSBB            = 0x09,

			LLS             = 0x0A,
			RLS             = 0x0B,
			ARS             = 0x0C,
			ROTL            = 0x0D,
			ROTR            = 0x0E,

			MUL             = 0x10,
			SMUL            = 0x11,
			DIV             = 0x12,
			SDIV            = 0x13,

			LOAD            = 0x20,
			LOADW           = 0x21,
			LOADB           = 0x22,

			STORE           = 0x24,
			STOREW          = 0x25,
			STOREB          = 0x26,
		};

		// 2 Parameters OpCodes *******************************************************

		/**
		 * 2 parameter OpCodes
		 */
		enum P2_OPCODE {
			MOV             = 0x00,
			SWP             = 0x01,

			SIGXB           = 0x02,
			SIGXW           = 0x03,

			//NOT             = 0x04,

			LOAD2           = 0x40,
			LOADW2          = 0x41,
			LOADB2          = 0x42,

			STORE2          = 0x44,
			STOREW2         = 0x45,
			STOREB2         = 0x46,

			IFEQ            = 0x20,
			IFNEQ           = 0x21,

			IFL             = 0x22,
			IFSL            = 0x23,
			IFLE            = 0x24,
			IFSLE           = 0x25,

			IFBITS          = 0x26,
			IFCLEAR         = 0x27,

			JMP2            = 0x60,
			CALL2           = 0x61,
		};

		// 1 Parameter OpCodes ********************************************************

		/**
		 * 1 Parameter OpCodes
		 */
		enum P1_OPCODE {

			XCHGB           = 0x00,
			XCHGW           = 0x01,

			GETPC           = 0x02,

			POP             = 0x09,
			PUSH            = 0x0A,

			JMP             = 0x18,
			CALL            = 0x19,

			RJMP            = 0x1A,
			RCALL           = 0x1B,

			INT             = 0x1C,
		};

		// 0 Parameters OpCodes ********************************************************

		/**
		 * 0 Paramaters OpCodes
		 */
		enum NP_OPCODE {
			SLEEP           = 0x00000000,

			RET             = 0x00000004,
			RFI             = 0x00000005,
		};


	} // End of namespace cpu
} // End of namespace vm

#endif // __TR3200_OPCODES__
