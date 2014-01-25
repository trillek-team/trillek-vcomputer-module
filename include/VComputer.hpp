#pragma once
/**
 * TR3200 VM - vcomputer.hpp
 * Virtual machine itself
 */

#ifndef __VM_HPP_
#define __VM_HPP_ 1

#include "Types.hpp"

#include "ICpu.hpp"
#include "Ram.hpp"
#include "IDevice.hpp"

#include <vector>
#include <algorithm>
#include <memory>

#include <cassert>

namespace vm {
using namespace vm::cpu;
using namespace vm::ram;

const unsigned MAX_N_DEVICES = 32;  /// Max number of devices attached

/**
 * Hardware Enumerator Commands
 */
enum HWN_CMD {
        GET_NUMBER = 0,
        GET_CLASS  = 1,
        GET_BUILDER  = 2,
        GET_ID  = 3,
        GET_VERSION  = 4,
        GET_JMP1  = 0x10,
        GET_JMP2  = 0x20,
};

template <typename CPU_t> 
class VirtualComputer {
public:

  /**
   * Creates a Virtual Computer
   * @param ram_size RAM size in BYTES
   */
  VirtualComputer (dword_t buildid = 0, std::size_t ram_size = 128*1024) : 
			buildid(buildid), ram(ram_size), cpu(ram), n_devices(0), enumerator(this), timers(this) {

		ram.AddBlock(&enumerator);  // Add Enumerator address handler
		ram.AddBlock(&timers);      // Add PIT address handler

		std::fill_n(devices, MAX_N_DEVICES, nullptr);
		std::fill_n(sdevices, MAX_N_DEVICES, nullptr);
		std::fill_n(idevices, MAX_N_DEVICES, nullptr);
	}

  ~VirtualComputer () {
	}

	/**
	 * Changes the build id (this not should happen when is running!)
	 */
	void BuildID (dword_t val) {
		this->buildid = val;
	}

	/**
	 * Return the BuildID
	 */
	dword_t BuildID () const {
		return this->buildid;
	}

  /**
   * Resets the virtual machine (but not clears RAM!)
   */
  void Reset() {
		cpu.Reset();
	}

  /**
   * Writes the ROM data to the internal array
   * @param *rom Ptr to the data to be copied to the ROM
   * @param rom_size Size of the ROM data that must be less or equal to 64KiB. Big sizes will be ignored
   */
  void WriteROM (const byte_t* rom, size_t rom_size) {
		assert (rom != nullptr);

		ram.WriteROM(rom, rom_size);
	}

  /**
   * Adds a Device to the virtual machine in a slot
   * @param slot Were plug the device
   * @param dev The device to be pluged in the slot
   * @return False if the slot have a device or the slot is invalid.
   */
  bool AddDevice (unsigned slot , IDevice& dev) {
		if (slot >= MAX_N_DEVICES)
			return false;

		if (devices[slot] != nullptr)
			return false;

		devices[slot] = &dev;
		if (dev.Flags() & DeviceFlags::WITH_INTERRUPTS) {
			idevices[slot] = &dev;
		}
		if (dev.Flags() & DeviceFlags::SYNC) {
			sdevices[slot] = &dev;
		}
		n_devices++;
		ram.AddBlock(dev.MemoryBlocks()); // Add Address handlerss

		return true;
	}

  /**
   * Remove a device from a virtual machine slot
   * @param slot Slot were unplug the device
	 * TODO Fix all!
   */
  void RemoveDevice (unsigned slot) {
		if (slot < MAX_N_DEVICES && devices[slot] != nullptr) {
			devices[slot] = nullptr;
			n_devices--;
			assert(n_devices >= 0);
		}
	}


  /**
   * Returns the actual CPU instance
   */
  const CPU_t& CPU () const {
    return cpu;
  }

  /**
   * Returns the actual RAM image
   */
  const Mem& RAM () const {
    return ram;
  }

  /**
   * Virtual Clock speed
   */
  unsigned Clock() const {
    return cpu.Clock();
  }


  /**
   * Executes one instruction
   * @param delta Number of milliseconds since the last call
   * @return number of cycles executed
   */
  unsigned Step( const double delta = 0) {
		unsigned cycles = cpu.Step();
		timers.Update(cycles);
	
		std::size_t i=0;
		dword_t msg;
		bool interrupted = timers.DoesInterrupt(msg);
		for (; i < MAX_N_DEVICES; i++) {
			// Does the sync job
			if (sdevices[i] != nullptr) {
				sdevices[i]->Tick(cycles, delta);
			}
			// Try to get the highest priority interrupt
			if (! interrupted && idevices[i] != nullptr && idevices[i]->DoesInterrupt(msg)) {
				interrupted = true;
			}
		}

		// Send the interrupt to the CPU if there is any
		if (interrupted) {
			cpu.ThrowInterrupt(msg);
		}

		return cycles;
	}

  /**
   * Executes N clock ticks
   * @param n nubmer of clock ticks, by default 1
   * @param delta Number of milliseconds since the last call
   */
  void Tick( unsigned n=1, const double delta = 0) {
		assert(n >0);

		cpu.Tick(n);
		timers.Update(n);
		
		std::size_t i=0;
		dword_t msg;
		bool interrupted = timers.DoesInterrupt(msg);
		for (; i < MAX_N_DEVICES; i++) {
			// Does the sync job
			if (sdevices[i] != nullptr) {
				sdevices[i]->Tick(n, delta);
			}
			// Try to get the highest priority interrupt
			if (! interrupted && idevices[i] != nullptr && idevices[i]->DoesInterrupt(msg)) {
				interrupted = true;
			}
		}

		// Send the interrupt to the CPU if there is any
		if (interrupted) {
			cpu.ThrowInterrupt(msg);
		}
		
	}

private:

	dword_t buildid;

	ram::Mem ram;					/// Handles the RAM mapings / access 
  CPU_t cpu; /// Virtual CPU

  IDevice* devices[MAX_N_DEVICES];	/// Devices atached to the virtual computer
  IDevice* sdevices[MAX_N_DEVICES]; /// Devices that run in sync with the VM clock
  IDevice* idevices[MAX_N_DEVICES]; /// Devices that can throw a interrupt
  unsigned n_devices;

  /**
   * Addres handler of Hardware enumerator
   * Also implementes the HWN itself as the HWN not does nothing outside responding commands
   */
  class HWN : public ram::AHandler {
  public:
    HWN (VirtualComputer* vm) {
			assert(vm != nullptr);

			this->vm = vm;
			this->begin = 0xFF000000;
			this->size = 0xC;
			ndev = 0;
			read = 0;
		}

    virtual ~HWN () {
		}

    byte_t RB (dword_t addr) {
			addr -= this->begin;
			if (addr == 0) {
				return read & 0xFF;

			}	else if (addr == 1) {
				return read >> 8;

			} else if (addr >= 4 && addr <= 7) {
				// Return CLK register
				switch (addr) {
					case 4:
						return vm->CPU().Clock() & 0xFF;

					case 5:
						return (vm->CPU().Clock() >> 8) & 0xFF;
					
					case 6:
						return (vm->CPU().Clock() >> 16) & 0xFF;
					
					case 7:
						return (vm->CPU().Clock() >> 24) & 0xFF;
				}
				
			} else if (addr >= 8 && addr <= 0xB) {
				// Return BUILID registers
				switch (addr) {
					case 8:
						return vm->buildid & 0xFF;

					case 9:
						return (vm->buildid >> 8) & 0xFF;
					
					case 10:
						return (vm->buildid >> 16) & 0xFF;
					
					case 11:
						return (vm->buildid >> 24) & 0xFF;
				}
				
				
			}
			return 0;
		}

    /**
     * Gets the commad value for the enumarator
     */
    void WB (dword_t addr, byte_t val) {
			addr -= this->begin;
			if (addr == 0) {
				ndev = val;
			} else {
				if (val == HWN_CMD::GET_NUMBER) { // Get number of devices commad
					read = vm->n_devices;
					return;
				}

				if (ndev >= MAX_N_DEVICES || vm->devices[ndev] == nullptr) {
					read = 0;   // Invalid device. Reads 0
					return;          
				}

				// Updates the read value
				switch (val) {
					case HWN_CMD::GET_CLASS : 
						read = vm->devices[ndev]->DevClass();
						break; 

					case HWN_CMD::GET_BUILDER : 
						read = vm->devices[ndev]->Builder();
						break; 

					case HWN_CMD::GET_ID : 
						read = vm->devices[ndev]->DevId();
						break; 

					case HWN_CMD::GET_VERSION : 
						read = vm->devices[ndev]->DevVer();
						break; 

					case HWN_CMD::GET_JMP1 : 
						read = vm->devices[ndev]->Jmp1();
						break; 

					case HWN_CMD::GET_JMP2 : 
						read = vm->devices[ndev]->Jmp2();
						break; 

					default:
						break;

				}
			}
		}
  
  private:
    VirtualComputer* vm;

    byte_t ndev;    /// Device Index of the HWN command
    word_t read;    /// Value to be read
  };

HWN enumerator;

  /**
   * Addres handler that setups the PIT (timers)
   */
  class PIT : public ram::AHandler {
  public:
    PIT (VirtualComputer* vm) : tmr0(0), tmr1(0), re0(0), re1(0), cfg(0) {
			assert(vm != nullptr);

			this->vm = vm;
			this->begin = 0xFF000040;
			this->size = 17;
		}

    virtual ~PIT () {
		}

    byte_t RB (dword_t addr) {
			addr &= 0x7F; // We only need to analize address 40 to 50
			switch (addr) {
				// Read TMR0_VAL
				case 0x40 :
					return (byte_t)tmr0;

				case 0x41 :
					return (byte_t)(tmr0>>8);

				case 0x42 :
					return (byte_t)(tmr0>>16);

				case 0x43 :
					return (byte_t)(tmr0>>24);

					// Read TMR0_RELOAD
				case 0x44 :
					return (byte_t)(re0);

				case 0x45 :
					return (byte_t)(re0>>8);

				case 0x46 :
					return (byte_t)(re0>>16);

				case 0x47 :
					return (byte_t)(re0>>24);

					// Read TMR1_VAL
				case 0x48 :
					return (byte_t)tmr1;

				case 0x49 :
					return (byte_t)(tmr1>>8);

				case 0x4A :
					return (byte_t)(tmr1>>16);

				case 0x4B :
					return (byte_t)(tmr1>>24);

					// Read TMR1_RELOAD
				case 0x4C :
					return (byte_t)(re1);

				case 0x4D :
					return (byte_t)(re1>>8);

				case 0x4E :
					return (byte_t)(re1>>16);

				case 0x4F :
					return (byte_t)(re1>>24);

					// Read TMR_CFG
				case 0x50 :
					return cfg;

				default:
					return 0;
			}
		}

    void WB (dword_t addr, byte_t val) {
			addr &= 0x7F; // We only need to analize address 40 to 50
			switch (addr) {
				// Write TMR0_RELOAD
				case 0x44 :
					re0 = (re0 & 0xFFFFFF00) | val;

				case 0x45 :
					re0 = (re0 & 0xFFFF00FF) | (val<<8);

				case 0x46 :
					re0 = (re0 & 0xFF00FFFF) | (val<<16);

				case 0x47 :
					re0 = (re0 & 0x00FFFFFF) | (val<<24);

					// Write TMR1_RELOAD
				case 0x4C :
					re1 = (re1 & 0xFFFFFF00) | val;

				case 0x4D :
					re1 = (re1 & 0xFFFF00FF) | (val<<8);

				case 0x4E :
					re1 = (re1 & 0xFF00FFFF) | (val<<16);

				case 0x4F :
					re1 = (re1 & 0x00FFFFFF) | (val<<24);

					// Write TMR_CFG
				case 0x50 :
					cfg = val;

				default:
					;
			}
		}

   
    /**
     * Update the timers and generate the interrupt if underflow hapens
     * @param n Number of cycles executed
     */
    void Update(unsigned n) {
			assert(n >0);

			dword_t tmp;
			if ((cfg & 1) != 0) {
				tmp = tmr0;
				tmr0 -= n;
				if (tmr0 > tmp) { // Underflow of TMR0
					tmr0 = re0 - (0xFFFFFFFF - tmr0);
					do_int_tmr0 = (cfg & 2) != 0;
				}
			}

			if ((cfg & 8) != 0) {
				tmp = tmr1;
				tmr1 -= n;
				if (tmr1 > tmp) { // Underflow of TMR1
					tmr1 = re1 - (0xFFFFFFFF - tmr1);
					do_int_tmr1 = (cfg & 16) != 0;
				}
			}

		}
	
		/**
		 * Checks if the device is trying to thorow a interrupt
		 * @param msg The interrupt message will be writen here
		 * @return True if is generating a new interrupt
		 */
		bool DoesInterrupt(dword_t& msg) {
			if ( ((cfg & 2) != 0) && do_int_tmr0) {
				msg = 0x0001;
				do_int_tmr0 = false;
				return true;

			} else if (((cfg & 16) != 0) && do_int_tmr1) {
				msg = 0x1001;
				do_int_tmr1 = false;
				return true;
			}
			return false;
		}

  private:
    VirtualComputer* vm;
      
    dword_t tmr0;     /// Timer 0
    dword_t tmr1;     /// Timer 1
    
    dword_t re0;      /// Reload value of Timer 0
    dword_t re1;      /// Reload value of Timer 1

    byte_t cfg;       /// Config byte of PIT

    bool do_int_tmr0; /// Try to thorow interrupt of TMR0 ?
    bool do_int_tmr1; /// Try to thorow interrupt of TMR1 ?
  };

PIT timers;

};

} // End of namespace vm

#endif // __VM_HPP_
