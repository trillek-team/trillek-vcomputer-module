/**
 * Trillek Virtual Computer - VComputer.cpp
 * Virtual Computer core
 */

#include "VComputer.hpp"
#include "VSFix.hpp"

#include <algorithm>
#include <cstdio>
#include <cassert>

namespace vm {
    using namespace vm::cpu;

    VComputer::VComputer (std::size_t ram_size ) :
        is_on(false), ram(nullptr), rom(nullptr), ram_size(ram_size), rom_size(0) {

            ram = new byte_t[ram_size];
            std::fill_n(ram, ram_size, 0);

            // Add timers addresses
            Range pit_range(0x11E000, 0x11E010);
            AddAddrListener(pit_range, &pit);

            // Add RNG address
            Range rng_range(0x11E040, 0x11E043);
            AddAddrListener(rng_range, &rng);

            // Add RTC address
            Range rtc_range(0x11E030, 0x11E036);
            AddAddrListener(rtc_range, &rtc);
        }

    VComputer::~VComputer () {
        if (ram != nullptr)
            delete[] ram;

        // Drops plugged devices
        for (unsigned i=0; i < MAX_N_DEVICES; i++) {
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

    bool VComputer::AddDevice (unsigned slot , std::shared_ptr<IDevice> dev) {
        if (slot >= MAX_N_DEVICES || std::get<0>(devices[slot])) {
            return false;
        }

        // TODO See were store the pointer or use a shared_ptr
        auto enumblk = new EnumAndCtrlBlk(slot, dev.get());
        std::get<2>(devices[slot]) = this->AddAddrListener(enumblk->GetRange(), enumblk);
        if (std::get<2>(devices[slot]) != -1) {
            std::get<0>(devices[slot]) = dev;
            dev->SetVComputer(this);
            std::get<1>(devices[slot]) = enumblk;
        } else { // Wops ! Problem!
            delete enumblk;
            return false;
        }

        return true;
    }

    std::shared_ptr<IDevice> VComputer::GetDevice (unsigned slot) {
        if (slot >= MAX_N_DEVICES || !std::get<0>(devices[slot])) {
            return nullptr;
        }

        return std::get<0>(devices[slot]);
    }

    void VComputer::RmDevice (unsigned slot) {
        if (slot < MAX_N_DEVICES && std::get<0>(devices[slot])) {
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

    void VComputer::SetROM (const byte_t* rom, std::size_t rom_size) {
        assert (rom != nullptr);
        assert (rom_size > 0);

        this->rom = rom;
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
            if ( !std::get<0>(devices[slot])) {
                continue;
            }
            std::get<0>(devices[slot])->Reset();
        }
    }

    void VComputer::On() {
        if (cpu && !is_on) { // Powering it wihtout cpu ?
            is_on = true;
            this->Reset(); // When we power on, we get a Reset!
        }
    }

    void VComputer::Off() {
        is_on = false;
    }

    unsigned VComputer::Update( const double delta) {
        assert (delta > 0);

        unsigned cycles = (vm::BaseClock * delta ) + 0.5f;
        // +0.5 for rounding bug in VS

        if (cycles <= 1) {
            cycles = 1;
        } else if (cycles >= 80000) {
            cycles = 80000;
        }

        this->Tick(cycles, delta);
        return cycles;
    }

    unsigned VComputer::Step( const double delta) {
        if (is_on) {
            unsigned cpu_ticks = cpu->Step();
            unsigned base_ticks = cpu_ticks * (BaseClock / cpu->Clock());
            unsigned dev_ticks = (base_ticks / 10); // Devices clock is at 100 KHz
            pit.Tick(dev_ticks, delta);

            word_t msg;
            bool interrupted = pit.DoesInterrupt(msg); // Highest priority interrupt
            if (interrupted) {
                if (cpu->SendInterrupt(msg)) { // Send the interrupt to the CPU
                    pit.IACK();
                }
            }

            for (std::size_t i=0; i < MAX_N_DEVICES; i++) {
                if (! std::get<0>(devices[i])) {
                    continue; // Slot without device
                }

                // Does the sync job
                if ( std::get<0>(devices[i])->IsSyncDev()) {
                    std::get<0>(devices[i])->Tick(dev_ticks, delta);
                }

                // Try to get the highest priority interrupt
                if (! interrupted && std::get<0>(devices[i])->DoesInterrupt(msg) ) {
                    interrupted = true;
                    if (cpu->SendInterrupt(msg)) { // Send the interrupt to the CPU
                        std::get<0>(devices[i])->IACK(); // Informs to the device that his interrupt has been accepted by the CPU
                    }
                }
            }

            return base_ticks;
        }

        return 0;
    }

    void VComputer::Tick( unsigned n, const double delta) {
        assert(n >0);
        if (is_on) {
            unsigned dev_ticks = n / 10; // Devices clock is at 100 KHz
            unsigned cpu_ticks = n / (BaseClock / cpu->Clock());

            cpu->Tick(cpu_ticks);
            pit.Tick(dev_ticks, delta);

            word_t msg;
            bool interrupted = pit.DoesInterrupt(msg); // Highest priority interrupt
            if (interrupted) {
                if (cpu->SendInterrupt(msg)) { // Send the interrupt to the CPU
                    pit.IACK();
                }
            }

            for (std::size_t i=0; i < MAX_N_DEVICES; i++) {
                if (! std::get<0>(devices[i])) {
                    continue; // Slot without device
                }

                // Does the sync job
                if ( std::get<0>(devices[i])->IsSyncDev()) {
                    std::get<0>(devices[i])->Tick(dev_ticks, delta);
                }

                // Try to get the highest priority interrupt
                if (! interrupted && std::get<0>(devices[i])->DoesInterrupt(msg) ) {
                    interrupted = true;
                    if (cpu->SendInterrupt(msg)) { // Send the interrupt to the CPU
                        std::get<0>(devices[i])->IACK(); // Informs to the device that his interrupt has been accepted by the CPU
                    }
                }
            }

        }
    }

    int32_t VComputer::AddAddrListener (const Range& range, AddrListener* listener) {
        assert(listener != nullptr);
        if (listeners.insert(std::make_pair(range, listener)).second ) { // Correct insertion
            return range.start;
        }
        return -1;
    }

    bool VComputer::RmAddrListener (int32_t id) {
        Range r(id);
        return listeners.erase(r) >= 1;
    }

} // End of namespace vm

