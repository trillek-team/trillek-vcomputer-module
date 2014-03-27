#include <emscripten.h>
#include <emscripten/bind.h>

#include <string>
#include <cstdio>

#include "VC.hpp"
#include "DisTR3200.hpp"
using namespace emscripten;

// Wrapper functions

void SetROM_ (vm::VComputer& arr ,long ptr, size_t rom_size) {
  auto p = (vm::byte_t *)ptr;
  arr.SetROM(p, rom_size);
}

void WriteTDATexture_(vm::dev::tda::TDAScreen& arr, long ptr) {
  auto p = (vm::dword_t *)ptr;
  vm::dev::tda::TDAtoRGBATexture(arr, p);
}

std::string Disassembly_(vm::VComputer& arr, long pos) {
  return vm::cpu::Disassembly(arr, pos);
}

EMSCRIPTEN_BINDINGS(tr3200_vm) {

    class_<vm::VComputer>("VComputer")
      .constructor<int>()
      //.function("SetCPU",     &vm::VComputer::SetCPU)
      //.function("RmCPU",      &vm::VComputer::RmCPU)
      //.function("AddDevice",  &vm::VComputer::AddDevice)
      //.function("GetDevice",  &vm::VComputer::GetDevice)
      //.function("RmDevice",   &vm::VComputer::RmDevice)
      .function("CPUClock",   &vm::VComputer::CPUClock)
      .function("SetROM",     &SetROM_)
      .function("Reset",      &vm::VComputer::Reset)
      .function("Step",       &vm::VComputer::Step)
      .function("Tick",       &vm::VComputer::Tick)
      .function("RamSize",    &vm::VComputer::RamSize)
      //.function("Ram",        &vm::VComputer::Ram)
      .function("Disassembly",&Disassembly_)
      ;
}


