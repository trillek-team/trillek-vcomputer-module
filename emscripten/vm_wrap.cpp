#include <emscripten.h>
#include <emscripten/bind.h>

#include <string>
#include <cstdio>

#include "vm.hpp"
using namespace emscripten;

/*
 * TODO CpuState:
      .field("r",           &vm::cpu::CpuState::r)

*/

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
  p[0] = 255;
  p[1] = 255;
  p[2] = 255;
  p[3] = 255;


  p[2600] = 255;
  p[2601] = 255;
  p[2602] = 255;
  p[2603] = 255;
}

EMSCRIPTEN_BINDINGS(rc3200_vm) {
    function("LoadROM", &vm::aux::LoadROM);
    
    value_object<vm::cpu::CpuState>("CpuState")
      .field("pc",          &vm::cpu::CpuState::pc)
      .field("int_msg",     &vm::cpu::CpuState::int_msg)
      .field("wait_cycles", &vm::cpu::CpuState::wait_cycles)
      .field("skiping",     &vm::cpu::CpuState::skiping)
      .field("sleeping",    &vm::cpu::CpuState::sleeping)
      .field("interrupt",   &vm::cpu::CpuState::interrupt)
      .field("iacq",        &vm::cpu::CpuState::iacq)
      .field("step_mode",   &vm::cpu::CpuState::step_mode)
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

    enum_<vm::keyboard::SCANCODES>("SCANCODES")
      .value("BACKSPACE", vm::keyboard::BACKSPACE)
      .value("RETURN", vm::keyboard::RETURN)
      .value("INSERT", vm::keyboard::INSERT)
      .value("DELETE", vm::keyboard::DELETE)
      .value("ESC", vm::keyboard::ESC)
      .value("ARROW_UP", vm::keyboard::ARROW_UP)
      .value("ARROW_DOWN", vm::keyboard::ARROW_DOWN)
      .value("ARROW_LEFT", vm::keyboard::ARROW_LEFT)
      .value("ARROW_RIGHT", vm::keyboard::ARROW_RIGHT)
      .value("SHIFT", vm::keyboard::SHIFT)
      .value("CONTROL", vm::keyboard::CONTROL)
      ;
      
}


