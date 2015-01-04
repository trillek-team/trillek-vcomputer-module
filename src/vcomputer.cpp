/**
 * \brief       Virtual Computer Core
 * \file        vcomputer.cpp
 * \copyright   The MIT License (MIT)
 *
 * Virtual Computer core
 */

#include "vcomputer.hpp"
#include "vs_fix.hpp"
#include "config.hpp"

#include <algorithm>
#include <cstdio>
#include <cassert>

namespace trillek {
namespace computer {


unsigned GetMajorVersion() {
    return MajorVersion;
}
unsigned GetMinorVersion() {
    return MinorVersion;
}
const char* GetBuildVersion() {
    return Build;
}

/* A function for aligned malloc that is portable */
static uint8_t *my_malloc(size_t size) {
    void *block = nullptr;
    int res = 0;

#if defined(_WIN32)
    /* A (void *) cast needed for avoiding a warning with MINGW :-/ */
    block = (void *)_aligned_malloc(size, 16);
#elif defined __APPLE__
    /* Mac OS X guarantees 16-byte alignment in small allocs */
    block = malloc(size);
#elif _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
    /* Platform does have an implementation of posix_memalign */
    res = posix_memalign(&block, 16, size);
#else
    block = malloc(size);
#endif  /* _WIN32 */

    if (res != 0) {
        return nullptr;
    }

    return (uint8_t *)block;
}


/* Release memory booked by my_malloc */
static void my_free(void *block) {
    assert(block != nullptr);
#if defined(_WIN32)
    _aligned_free(block);
#else
    free(block);
#endif  /* _WIN32 */
}



VComputer::VComputer (std::size_t ram_size ) :
    is_on(false), ram(nullptr), rom(nullptr), ram_size(ram_size), rom_size(0), breaking(false), recover_break(false) {

    ram = my_malloc(ram_size);  //new byte_t[ram_size];
    assert(ram != nullptr);
    std::fill_n(ram, ram_size, 0);

    // Add timers addresses
    Range pit_range(0x11E000, 0x11E010);
    AddAddrListener(pit_range, &pit);

    // Add RNG address
    Range rng_range(0x11E040, 0x11E043);
    AddAddrListener(rng_range, &rng);

    // Add Beeper address
    Range beeper_range(0x11E020, 0x11E021);
    AddAddrListener(beeper_range, &beeper);

    // Add RTC address
    Range rtc_range(0x11E030, 0x11E036);
    AddAddrListener(rtc_range, &rtc);
}

VComputer::~VComputer () {
    if (ram != nullptr) {
        my_free((void*)ram);
        //delete[] ram;
    }

    // Drops plugged devices
    for (unsigned i = 0; i < MAX_N_DEVICES; i++) {
        this->RmDevice(i);
    }
}

void VComputer::SetCPU (std::unique_ptr<ICPU> cpu) {
    this->cpu = std::move(cpu);
    this->cpu->SetVComputer(this);
}

std::unique_ptr<ICPU> VComputer::RmCPU () {
    this->cpu->SetVComputer(nullptr);
    return std::move(cpu);
}

bool VComputer::haveCpu() const {
    return true && cpu; // Because compiler not like to return directly toe condition
}

bool VComputer::AddDevice (unsigned slot, std::shared_ptr<Device> dev) {
    if ( slot >= MAX_N_DEVICES || std::get<0>(devices[slot]) ) {
        return false;
    }

    // TODO See were store the pointer or use a shared_ptr
    auto enumblk = new EnumAndCtrlBlk( slot, dev.get() );
    std::get<2>(devices[slot]) = this->AddAddrListener(enumblk->GetRange(), enumblk);
    if (std::get<2>(devices[slot]) != -1) {
        std::get<0>(devices[slot]) = dev;
        dev->SetVComputer(this);
        std::get<1>(devices[slot]) = enumblk;
    }
    else {
        // Wops ! Problem!
        delete enumblk;
        return false;
    }

    return true;
} // AddDevice

std::shared_ptr<Device> VComputer::GetDevice (unsigned slot) {
    if ( slot >= MAX_N_DEVICES || !std::get<0>(devices[slot]) ) {
        return nullptr;
    }

    return std::get<0>(devices[slot]);
}

void VComputer::RmDevice (unsigned slot) {
    if ( slot < MAX_N_DEVICES && std::get<0>(devices[slot]) ) {
        std::get<0>(devices[slot])->SetVComputer(nullptr);
        std::get<0>(devices[slot]).reset(); // Cleans the slot
        delete std::get<1>(devices[slot]);
        std::get<1>(devices[slot]) = nullptr;
        std::get<2>(devices[slot]) = -1;
    }
}

unsigned VComputer::CPUClock() const {
    if (cpu) {
        return cpu->Clock();
    }
    return 0;
}

void VComputer::GetState (void* ptr, std::size_t size) const {
    if (cpu) {
        return cpu->GetState(ptr, size);
    }
}

void VComputer::SetROM (const Byte* rom, std::size_t rom_size) {
    assert (rom != nullptr);
    assert (rom_size > 0);

    this->rom      = rom;
    this->rom_size = (rom_size > MAX_ROM_SIZE) ? MAX_ROM_SIZE : rom_size;
}

void VComputer::Reset() {
    if (cpu) {
        cpu->Reset();
    }

    // Reset embed devices
    pit.Reset();
    rng.Reset();

    // Reset devices
    for (unsigned slot = 0; slot < MAX_N_DEVICES; slot++) {
        if ( !std::get<0>(devices[slot]) ) {
            continue;
        }
        std::get<0>(devices[slot])->Reset();
    }


    // Cleat Break status
    breaking = false;
} // Reset

void VComputer::On() {
    // Powering it wihtout cpu ?
    if (cpu && !is_on) {
        std::fill_n(ram, ram_size, 0);
        is_on = true;
        this->Reset(); // When we power on, we get a Reset!
    }
}

void VComputer::Off() {
    is_on = false;
}

bool VComputer::isOn() const {
    return is_on && cpu;
}

unsigned VComputer::Update( const double delta) {
    assert (delta > 0);

    unsigned cycles = (computer::BaseClock * delta ) +0.5f;
    // +0.5 for rounding bug in VS

    if (cycles <= 1) {
        cycles = 1;
    }
    else if (cycles >= 80000) {
        cycles = 80000;
    }

    this->Tick(cycles, delta);
    return cycles;
} // Update

unsigned VComputer::Step( const double delta) {
    if (is_on) {
        unsigned cpu_ticks = cpu->Step();

        #ifdef BRKPOINTS
        if (breaking) {
            return 0; // We not executed yet the instruction!
        }
        #endif

        unsigned base_ticks = cpu_ticks * ( BaseClock / cpu->Clock() );
        unsigned dev_ticks  = (base_ticks / 10); // Devices clock is at
                                                 // 100 KHz
        pit.Tick(dev_ticks, delta);

        Word msg;
        bool interrupted = pit.DoesInterrupt(msg); // Highest priority
                                                   // interrupt
        if (interrupted) {
            if ( cpu->SendInterrupt(msg) ) {
                // Send the interrupt to the CPU
                pit.IACK();
            }
        }

        for (std::size_t i = 0; i < MAX_N_DEVICES; i++) {
            if ( !std::get<0>(devices[i]) ) {
                continue; // Slot without device
            }

            // Does the sync job
            if ( std::get<0>(devices[i])->IsSyncDev() ) {
                std::get<0>(devices[i])->Tick(dev_ticks, delta);
            }

            // Try to get the highest priority interrupt
            if ( !interrupted && std::get<0>(devices[i])->DoesInterrupt(msg) ) {
                interrupted = true;
                if ( cpu->SendInterrupt(msg) ) {
                    // Send the interrupt to the CPU
                    std::get<0>(devices[i])->IACK(); // Informs to the device
                                                     // that his interrupt
                                                     // has been accepted by
                                                     // the CPU
                }
            }
        }

        return base_ticks;
    }

    return 0;
} // Step

void VComputer::Tick( unsigned n, const double delta) {
    assert(n > 0);
    if (is_on) {
        unsigned dev_ticks = n / 10; // Devices clock is at 100 KHz
        unsigned cpu_ticks = n / ( BaseClock / cpu->Clock() );

        cpu->Tick(cpu_ticks);
        // TODO ICPU.Tick should return the number of cycles that executed,
        // so we can accrutraly execute the apropaite number of Device
        // cycles if a breakpoint happens
        pit.Tick(dev_ticks, delta);

        Word msg;
        bool interrupted = pit.DoesInterrupt(msg); // Highest priority
                                                   // interrupt
        if (interrupted) {
            if ( cpu->SendInterrupt(msg) ) {
                // Send the interrupt to the CPU
                pit.IACK();
            }
        }

        for (std::size_t i = 0; i < MAX_N_DEVICES; i++) {
            if ( !std::get<0>(devices[i]) ) {
                continue; // Slot without device
            }

            // Does the sync job
            if ( std::get<0>(devices[i])->IsSyncDev() ) {
                std::get<0>(devices[i])->Tick(dev_ticks, delta);
            }

            // Try to get the highest priority interrupt
            if ( !interrupted && std::get<0>(devices[i])->DoesInterrupt(msg) ) {
                interrupted = true;
                if ( cpu->SendInterrupt(msg) ) {
                    // Send the interrupt to the CPU
                    std::get<0>(devices[i])->IACK(); // Informs to the device
                                                     // that his interrupt
                                                     // has been accepted by
                                                     // the CPU
                }
            }
        }
    }
} // Tick

int32_t VComputer::AddAddrListener (const Range& range, AddrListener* listener) {
    assert(listener != nullptr);
    if (listeners.insert( std::make_pair(range, listener) ).second ) {
        // Correct insertion
        return range.start;
    }
    return -1;
}

bool VComputer::RmAddrListener (int32_t id) {
    Range r(id);

    return listeners.erase(r) >= 1;

}

} // End of namespace computer
} // End of namespace trillek
