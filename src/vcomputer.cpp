/**
 * RC3200 VM - vcomputer.cpp
 * Virtual machine itself
 */

#include "vcomputer.hpp"

namespace vm {
using namespace vm::cpu;
using namespace vm::ram;

VirtualComputer::VirtualComputer (std::size_t ram_size) : cpu(ram_size), n_devices(0), enumerator(this), timers(this) {
  cpu.ram.AddBlock(&enumerator);  // Add Enumerator address handler
  cpu.ram.AddBlock(&timers);      // Add PIT address handler
  std::fill_n(devices, MAX_N_DEVICES, nullptr);
}

VirtualComputer::~VirtualComputer () {
}

void VirtualComputer::Reset() {
  cpu.Reset();
}

void VirtualComputer::WriteROM (const byte_t* rom, size_t rom_size) {
  assert (rom != nullptr);

  cpu.ram.WriteROM(rom, rom_size);
}

bool VirtualComputer::AddDevice (unsigned slot , IDevice& dev) {
  if (slot >= MAX_N_DEVICES)
    return false;

  if (devices[slot] != nullptr)
    return false;

  devices[slot] = &dev;
  n_devices++;
  cpu.ram.AddBlock(dev.MemoryBlocks()); // Add Address handlerss

  return true;
}

void VirtualComputer::RemoveDevice (unsigned slot) {
  if (slot < MAX_N_DEVICES && devices[slot] != nullptr) {
    devices[slot] = nullptr;
    n_devices--;
    assert(n_devices >= 0);
  }
}

unsigned VirtualComputer::Step( const double delta) {
  auto cycles = cpu.Step();
  timers.Update(cycles);
  
  for (std::size_t i=0; i < MAX_N_DEVICES; i++) {
    if (devices[i] != nullptr) {
      devices[i]->Tick(cpu, cycles, delta);
    }
  }
  return cycles;
}

void VirtualComputer::Tick( unsigned n, const double delta) {
  assert(n >0);

  cpu.Tick(n);
  timers.Update(n);
  
  for (std::size_t i=0; i < MAX_N_DEVICES; i++) {
    if (devices[i] != nullptr) {
      devices[i]->Tick(cpu, n, delta);
    }
  }
}

// HWN class inside of VirtualComputer class

VirtualComputer::HWN::HWN (VirtualComputer* vm) {
  assert(vm != nullptr);
  
  this->vm = vm;
  this->begin = 0xFF000000;
  this->size = 2;
  ndev = 0;
  read = 0;
}

VirtualComputer::HWN::~HWN () {
}

byte_t VirtualComputer::HWN::RB (dword_t addr) {
  addr -= this->begin;
  if (addr == 0)
    return read & 0xFF;
  else
    return read >> 8;
}

/**
 * Gets the commad value for the enumarator
 */
void VirtualComputer::HWN::WB (dword_t addr, byte_t val) {
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

// PIT class inside of VirtualComputer class

VirtualComputer::PIT::PIT (VirtualComputer* vm) : tmr0(0), tmr1(0), re0(0), re1(0), cfg(0) {
  assert(vm != nullptr);

  this->vm = vm;
  this->begin = 0xFF000040;
  this->size = 17;
}

VirtualComputer::PIT::~PIT () {
}

byte_t VirtualComputer::PIT::RB (dword_t addr) {
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

void VirtualComputer::PIT::WB (dword_t addr, byte_t val) {
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
void VirtualComputer::PIT::Update(unsigned n) {
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

  if (((cfg & 2) != 0) && do_int_tmr0) { // Try to throw TMR0 interrupt
    do_int_tmr0 = ! vm->cpu.ThrowInterrupt(0x0001);
  } else if (((cfg & 16) != 0) && do_int_tmr1) { // Try to thorow TMR1 interrupt 
    do_int_tmr1 = ! vm->cpu.ThrowInterrupt(0x1001);
  }
}


} // End of namespace vm

