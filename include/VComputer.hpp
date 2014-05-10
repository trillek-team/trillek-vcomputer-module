/**
 * \brief       Virtual Computer Core
 * \file        VComputer.hpp
 * \copyright   The MIT License (MIT)
 *
 * Virtual Computer core
 */
#ifndef __VCOMPUTER_HPP_
#define __VCOMPUTER_HPP_ 1

#include "Types.hpp"

#include "ICPU.hpp"
#include "IDevice.hpp"
#include "AddrListener.hpp"
#include "EnumAndCtrlBlk.hpp"
#include "devices/Timer.hpp"
#include "devices/RNG.hpp"
#include "devices/RTC.hpp"
#include "devices/Beeper.hpp"

#include <map>
#include <set>
#include <memory>
#include <cassert>

namespace vm {
using namespace vm::cpu;

const unsigned MAX_N_DEVICES = 32;          /// Max number of devices
                                            // attached
const std::size_t MAX_ROM_SIZE = 32*1024;   /// Max ROM size
const std::size_t MAX_RAM_SIZE = 1024*1024; /// Max RAM size

const unsigned EnumCtrlBlkSize = 20; /// Enumeration and Control registers
                                     // blk size

const unsigned BaseClock = 1000000; /// Computer Base Clock rate

class EnumAndCtrlBlk;

class VComputer {
public:

    /**
     * Creates a Virtual Computer
     * \param ram_size RAM size in BYTES
     */
    VComputer (std::size_t ram_size = 128*1024);

    ~VComputer ();

    /**
     * Sets the CPU of the computer
     * \param cpu New CPU in the computer
     */
    void SetCPU (std::unique_ptr<ICPU> cpu);

    /**
     * Removes the CPU of the computer
     * \return Returns the ICPU
     */
    std::unique_ptr<ICPU> RmCPU ();

    /**
     * Adds a Device to a slot
     * \param slot Were plug the device
     * \param dev The device to be pluged in the slot
     * \return False if the slot have a device or the slot is invalid.
     */
    bool AddDevice (unsigned slot, std::shared_ptr<IDevice> dev);

    /**
     * Gets the Device plugged in the slot
     */
    std::shared_ptr<IDevice> GetDevice (unsigned slot);

    /**
     * Remove a device from a slot
     * \param slot Slot were unplug the device
     */
    void RmDevice (unsigned slot);

    /**
     * CPU clock speed in Hz
     */
    unsigned CPUClock () const;

    /**
     * Writes a copy of CPU state in a chunk of memory pointer by ptr.
     * \param ptr Pointer were to write
     * \param size Size of the chunk of memory were can write. If is
     * sucesfull, it will be set to the size of the write data.
     */
    void GetState (void* ptr, std::size_t size) const;

    /**
     * Gets a pointer were is stored the ROM data
     * \param *rom Ptr to the ROM data
     * \param rom_size Size of the ROM data that must be less or equal to 32KiB.
     *******Big sizes will be ignored
     */
    void SetROM (const byte_t* rom, std::size_t rom_size);

    /**
     * Resets the virtual machine (but not clears RAM!)
     */
    void Reset ();

    /**
     * Power On the computer
     */
    void On ();

    /**
     * Power Off the computer
     */
    void Off ();

    /**
     * Executes the apropaited number of Virtual Computer base clock cycles
     * in function of the elapsed time since the last call (delta time)
     *
     * \param delta Number of seconds since the last call
     * \return Number of base clock cycles executed
     */
    unsigned Update ( const double delta);

    /**
     * Executes one instruction
     * \param delta Number of seconds since the last call
     * \return number of base clock ticks needed
     */
    unsigned Step ( const double delta = 0);

    /**
     * Executes N clock ticks
     * \param n nubmer of base clock ticks, by default 1
     * \param delta Number of seconds since the last call
     */
    void Tick ( unsigned n = 1, const double delta = 0);

    byte_t ReadB (dword_t addr) const {
        addr = addr & 0x00FFFFFF; // We use only 24 bit addresses

        if ( !(addr & 0xF00000 ) ) {
            // RAM address (0x000000-0x0FFFFF)
            return ram[addr];
        }

        if ( (addr & 0xFF0000) == 0x100000 ) {
            // ROM (0x100000-0x10FFFF)
            return rom[addr & 0x00FFFF];
        }

        Range r(addr);
        auto search = listeners.find(r);
        if ( search != listeners.end() ) {
            return search->second->ReadB(addr);
        }

        return 0;
    } // ReadB

    word_t ReadW (dword_t addr) const {
        addr = addr & 0x00FFFFFF; // We use only 24 bit addresses
        size_t tmp;

        if ( !(addr & 0xF00000 ) ) {
            // RAM address
            tmp = ( (size_t)ram ) + addr;
            return ( (word_t*)tmp )[0];
        }

        if ( (addr & 0xFF0000) == 0x100000 ) {
            // ROM (0x100000-0x10FFFF)
            addr &= 0x00FFFF; // Dirty tricks with pointers
            tmp   = ( (size_t)rom ) + addr;
            return ( (word_t*)tmp )[0];
        }

        Range r(addr);
        auto search = listeners.find(r);
        if ( search != listeners.end() ) {
            return search->second->ReadW(addr);
        }

        return 0;
    } // ReadW

    dword_t ReadDW (dword_t addr) const {
        addr = addr & 0x00FFFFFF; // We use only 24 bit addresses
        size_t tmp;

        if ( !(addr & 0xF00000 ) ) {
            // RAM address
            tmp = ( (size_t)ram ) + addr;
            return ( (dword_t*)tmp )[0];
        }

        if ( (addr & 0xFF0000) == 0x100000 ) {
            // ROM (0x100000-0x10FFFF)
            addr &= 0x00FFFF; // Dirty tricks with pointers
            tmp   = ( (size_t)rom ) + addr;
            return ( (dword_t*)tmp )[0];
        }

        Range r(addr);
        auto search = listeners.find(r);
        if ( search != listeners.end() ) {
            return search->second->ReadDW(addr);
        }

        return 0;
    } // ReadDW

    void WriteB (dword_t addr, byte_t val) {
        addr = addr & 0x00FFFFFF; // We use only 24 bit addresses

        if (addr < ram_size) {
            // RAM address
            ram[addr] = val;
        }

        Range r(addr);
        auto search = listeners.find(r);
        if ( search != listeners.end() ) {
            search->second->WriteB(addr, val);
        }
    } // WriteB

    void WriteW (dword_t addr, word_t val) {
        size_t tmp;

        addr = addr & 0x00FFFFFF; // We use only 24 bit addresses

        if (addr < ram_size-1 ) {
            // RAM address
            tmp                 = ( (size_t)ram ) + addr;
            ( (word_t*)tmp )[0] = val;
        }
        // TODO What hapens when there is a write that falls half in RAM and
        // half outside ?
        // I actually forbid these cases to avoid buffer overun, but should be
        // allowed and only use the apropiate portion of the data in the RAM.

        Range r(addr);
        auto search = listeners.find(r);
        if ( search != listeners.end() ) {
            search->second->WriteW(addr, val);
        }
    } // WriteW

    void WriteDW (dword_t addr, dword_t val) {
        size_t tmp;

        addr = addr & 0x00FFFFFF; // We use only 24 bit addresses

        if (addr < ram_size-3 ) {
            // RAM address
            tmp                  = ( (size_t)ram ) + addr;
            ( (dword_t*)tmp )[0] = val;
        }
        // TODO What hapens when there is a write that falls half in RAM and
        // half outside ?

        Range r(addr);
        auto search = listeners.find(r);
        if ( search != listeners.end() ) {
            search->second->WriteDW(addr, val);
        }
    } // WriteDW

    /**
     * Adds an AddrListener to the computer
     * \param range Range of addresses that the listerner listens
     * \param listener AddrListener using these range
     * \return And ID oif the listener or -1 if can't add the listener
     */
    int32_t AddAddrListener (const Range& range, AddrListener* listener);

    /**
     * Removes an AddrListener from the computer
     * \param id ID of the address listener to remove (ID from AddAddrListener)
     * \return True if can find these listener and remove it.
     */
    bool RmAddrListener (int32_t id);

    /**
     * Sizeo of the RAM in bytes
     */
    std::size_t RamSize () const {
        return ram_size;
    }

    /**
     * Returns a pointer to the RAM for reading raw values from it
     * Use only for GetState methods or dump a snapshot of the computer state
     */
    const byte_t* Ram () const {
        return ram;
    }

    /**
     * Returns a pointer to the RAM for writing raw values to it
     * Use only for SetState methods or load a snapshot of the computer state
     */
    byte_t* Ram() {
        return ram;
    }

    /**
     * /brief Assing a function to be called when Beeper freq is changed
     * /param f_changed function to be called
     */
    void SetFreqChangedCB (std::function<void(dword_t freq)> f_changed) {
        beeper.SetFreqChangedCB(f_changed);
    }

    /**
     * Add a breakpoint at the desired address
     * \param addr Address were will be the breakpoint
     */
    void SetBreakPoint (dword_t addr) {
        breakpoints.insert(addr);
    }

    /**
     * Erase a breakpoint at the desired address
     * \param addr Address were will be the breakpoint
     */
    void RmBreakPoint (dword_t addr) {
        breakpoints.erase(addr);
    }

    /**
     * Remove all breakpoints
     */
    void ClearBreakPoints () {
        breakpoints.clear();
    }

    /**
     * Check if there isa breakpoint at an particular address
     * \param addr Address to verify
     * \return True if there is a breakpoint in these address
     */
    bool isBreakPoint (dword_t addr) {
        if ( breakpoints.find(addr) != breakpoints.end() ) {
            last_break = addr;
            breaking   = true;
            return true;
        }

        if (recover_break) {
            // Recover temporaly removed breakpoint
            SetBreakPoint(last_break);
            recover_break = false;
        }

        return false;
    } // isBreakPoint

    /**
     * Check if the Virtual Computer is halted by a breakpoint
     * \return True if a breakpoint happened
     */
    bool isHalted () {
        return breaking;
    }

    /**
     * Allows to continue if a Breakpoint happened
     */
    void Resume () {
        if (breaking) {
            breaking = false;
            // Temporaly, we remove the last break point
            RmBreakPoint(last_break);
            recover_break = true;
        }
    }

private:

    bool is_on;                               /// Is PowerOn the computer ?
    byte_t* ram;                              /// Computer RAM
    const byte_t* rom;                        /// Computer ROM chip (could be
                                              // shared between some
                                              // VComputers)
    std::size_t ram_size;                     /// Computer RAM size
    std::size_t rom_size;                     /// Computer ROM size
    std::unique_ptr<ICPU> cpu;                /// Virtual CPU
    device_t devices[MAX_N_DEVICES];          /// Devices atached to the
                                              // virtual computer
    std::map<Range, AddrListener*> listeners; /// Container of AddrListeners

    Timer pit;     /// Programable Interval Timer
    RNG rng;       /// Random Number Generator
    RTC rtc;       /// Real Time Clock
    Beeper beeper; /// Real Time Clock

    std::set<dword_t> breakpoints; /// Breakpoints list
    bool breaking;                 /// The Virtual Computer is halted in a
                                   // BreakPoint ?

    dword_t last_break; /// Address tof the last breakpoint finded
    bool recover_break; /// Flag to know if a recovered the temporaly erases
                        // break
};
} // End of namespace vm

#endif // __VCOMPUTER_HPP_
