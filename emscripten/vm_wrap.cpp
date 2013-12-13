#include <emscripten/bind.h>

#include "vm.hpp"
using namespace emscripten;

int DumbTest () {
  return 1;
}
/*
 * TODO CpuState:
      .field("r",           &vm::cpu::CpuState::r)

*/

bool AddCDA_(vm::VirtualComputer& arr, unsigned slot, vm::cda::CDA& d ) {
  return arr.AddDevice(slot, d);
}

EMSCRIPTEN_BINDINGS(rc3200_vm) {
    function("DumbTest", &DumbTest);
    
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
      .function("WriteROM",   &vm::VirtualComputer::WriteROM, allow_raw_pointers())
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
      .function("ToRGBATexture",  &vm::cda::CDA::ToRGBATexture, allow_raw_pointers())
      ;

}


