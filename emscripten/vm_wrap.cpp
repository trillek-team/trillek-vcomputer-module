#include <emscripten.h>
#include <emscripten/bind.h>

#include <string>
#include <cstdio>

#include "VM.hpp"
using namespace emscripten;

// Wrapper functions

vm::dword_t r_(vm::cpu::CpuState& arr, unsigned n) {
  if (n < vm::cpu::TR3200_NGPRS) {
    return arr.r[n];
  }
  return -1;
}

vm::dword_t pc_(vm::cpu::CpuState& arr) {
  return arr.pc;
}


void WriteROM_ (vm::VirtualComputer& arr,long ptr, size_t rom_size) {
  auto p = (vm::byte_t *)ptr;
  arr.WriteROM(p, rom_size);
}


bool AddGKey_(vm::VirtualComputer& arr, unsigned slot, vm::keyboard::GKeyboard& d ) {
  return arr.AddDevice(slot, d);
}

bool AddCDA_(vm::VirtualComputer& arr, unsigned slot, vm::cda::CDA& d ) {
  return arr.AddDevice(slot, d);
}

void WriteTexture_(vm::cda::CDA& arr, long ptr) {
  auto p = (vm::byte_t *)ptr;
  arr.ToRGBATexture((vm::dword_t*)p); // TODO Perhaps we need a special function for this case
}

std::string Disassembly_(vm::VirtualComputer& arr) {
  return vm::cpu::Disassembly(arr.RAM(), arr.CPUState().pc);
}

EMSCRIPTEN_BINDINGS(rc3200_vm) {
    function("LoadROM",     &vm::aux::LoadROM);
    function("Register",    &r_);
    
    class_<vm::cpu::CpuState>("CpuState")
      .constructor<>()
      .function("R",          &r_)
      .function("PC",         &pc_)
      ;

    class_<vm::VirtualComputer>("VirtualComputer")
      .constructor<int>()
      .function("Reset",      &vm::VirtualComputer::Reset)
      .function("WriteROM",   &WriteROM_)
      .function("AddKeyboard",&AddGKey_)
      .function("AddCDA",     &AddCDA_)
      .function("RemoveDevice",  &vm::VirtualComputer::RemoveDevice)
      .function("CPUState",   &vm::VirtualComputer::CPUState)
      .function("Clock",      &vm::VirtualComputer::Clock)
      .function("Step",       &vm::VirtualComputer::Step)
      .function("Tick",       &vm::VirtualComputer::Tick)
      .function("Disassembly",&Disassembly_)
      ;

    class_<vm::cda::CDA>("CDA")
      .constructor<int, int>()
      .function("VideoMode",  &vm::cda::CDA::VideoMode)
      .function("isTextMode", &vm::cda::CDA::isTextMode)
      .function("isUserPalette",  &vm::cda::CDA::isUserPalette)
      .function("isUserFont", &vm::cda::CDA::isUserFont)
      .function("VSync",      &vm::cda::CDA::VSync)
      .function("ToRGBATexture",  &WriteTexture_, allow_raw_pointers())
      ;
    
    class_<vm::keyboard::GKeyboard>("GKeyboard")
      .constructor<int, int>()
      .function("PushKeyEvent",  &vm::keyboard::GKeyboard::PushKeyEvent)
      ;

    enum_<vm::keyboard::KEYCODES>("KEYCODES")
      .value("BACKSPACE",   vm::keyboard::KEY_BACKSPACE)
      .value("RETURN",      vm::keyboard::KEY_RETURN)
      .value("INSERT",      vm::keyboard::KEY_INSERT)
      .value("DELETE",      vm::keyboard::KEY_DELETE)
      .value("ESC",         vm::keyboard::KEY_ESC)
      .value("ARROW_UP",    vm::keyboard::KEY_ARROW_UP)
      .value("ARROW_DOWN",  vm::keyboard::KEY_ARROW_DOWN)
      .value("ARROW_LEFT",  vm::keyboard::KEY_ARROW_LEFT)
      .value("ARROW_RIGHT", vm::keyboard::KEY_ARROW_RIGHT)
      .value("SHIFT",       vm::keyboard::KEY_SHIFT)
      .value("ALT_GR",      vm::keyboard::KEY_ALT_GR)
      .value("CONTROL",     vm::keyboard::KEY_CONTROL)
      .value("UNKNOW",      vm::keyboard::KEY_UNKNOW)
      ;
      
}


