#pragma once
/**
 * Trillek Virtual Computer - DummyDevice.hpp
 * DummyDevice for testing
 */

#ifndef __DUMMY_DEV_HPP__
#define __DUMMY_DEV_HPP__ 1

#include "Types.hpp"
#include "VComputer.hpp"

namespace vm {
	
	/** 
   * DummyDevice for testing
	 */
	class DummyDevice : public IDevice {
		public:

			DummyDevice () {
			}
			
			virtual ~DummyDevice() {
			}

			/**
			 * Sends (writes to CMD register) a command to the device
			 * @param cmd Command value to send
			 */
			virtual void SendCMD (word_t cmd) {
				a = cmd;
			}

			virtual void A (word_t cmd) { a = cmd; }
			virtual void B (word_t cmd) { b = cmd; }
			virtual void C (word_t cmd) { c = cmd; }
			virtual void D (word_t cmd) { d = cmd; }
			virtual void E (word_t cmd) { e = cmd;}

			virtual word_t A () { return a; }
			virtual word_t B () { return b; }
			virtual word_t C () { return c; }
			virtual word_t D () { return d; }
			virtual word_t E () { return e; }

			/**
			 * Device Type
			 */
			virtual byte_t DevType() const {
				return 0;
			}

			/**
			 * Device SubType
			 */
			virtual byte_t DevSubType() const {
				return 1;
			}

			/**
			 * Device ID
			 */
			virtual byte_t DevID() const {
				return 0x5A;
			}

			/**
			 * Device Vendor ID
			 */
			virtual dword_t DevVendorID() const {
				return 0xBEEF55AA;
			}
			
			virtual void GetState (void* ptr, std::size_t& size) const {
			}

			virtual bool SetState (const void* ptr, std::size_t size) {
				return true;
			}

			word_t a, b, c, d, e;

	};


} // End of namespace vm

#endif

