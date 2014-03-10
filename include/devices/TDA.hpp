#pragma once
/**
 * Trillek Virtual Computer - TDA.hpp
 * Nya Elektriska Text Display Adapter
 */

#ifndef __TDA_DEV_HPP__
#define __TDA_DEV_HPP__ 1

#include "Types.hpp"
#include "VComputer.hpp"

namespace vm {
	namespace dev {
		/**
		 * Text Generator Adapter
		 * Text only video card
		 */
		class TDADev : public IDevice {
			public:

				TDADev () : buffer_ptr(0), font_ptr(0), vsync_msg(0) {
				}

				virtual ~TDADev() {
				}

				/**
				 * Sends (writes to CMD register) a command to the device
				 * @param cmd Command value to send
				 */
				virtual void SendCMD (word_t cmd) {
					dword_t tmp;
					switch (cmd) {
						case 0x0000: // Map Buffer
							tmp = ((b << 16) | a);
							if (tmp < vcomp->RamSize()) {
								buffer_ptr = tmp;
							}
							break;

						case 0x0001: // Map Font
							tmp = ((b << 16) | a);
							if (tmp < vcomp->RamSize()) {
								font_ptr = tmp;
							}
							break;

						case 0x0002: // Set Int
							vsync_msg = a;
							break;

						default:
							break;
					}
				}

				virtual void A (word_t val) { a = val; }
				virtual void B (word_t val) { b = val; }

				virtual word_t A () { return a; }
				virtual word_t B () { return b; }

				/**
				 * Device Type
				 */
				virtual byte_t DevType() const {
					return 0x0E; // Graphics device
				}

				/**
				 * Device SubType
				 */
				virtual byte_t DevSubType() const {
					return 0x01; // TDA compatible
				}

				/**
				 * Device ID
				 */
				virtual byte_t DevID() const {
					return 0x01; // Nya Elesktriska TDA
				}

				/**
				 * Device Vendor ID
				 */
				virtual dword_t DevVendorID() const {
					return 0x1C6C8B36; // Nya Elekstrika
				}

			protected:
				dword_t buffer_ptr;
				dword_t font_ptr;
				word_t vsync_msg;
				word_t a, b;

		};

	} // End of namespace dev
} // End of namespace vm

#endif

