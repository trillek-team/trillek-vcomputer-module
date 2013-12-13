#include <emscripten/bind.h>

#include "vm.hpp"
using namespace emscripten;

int DumbTest () {
  return 1;
}
/*
 * TODO CpuState:
      .field("r",           &vm::cpu::CpuState::r)

    class IDeviceWrapper : public wrapper<vm::IDevice> {
      EMSCRIPTEN_WRAPPER(IDeviceWrapper);
      vm::byte_t DevClass() const {
          return call<int>("DevClass");
      }
    };
    class_<vm::IDevice>("IDevice")
      .constructor<int, int>()
      .function("DevClass", &vm::IDevice::DevClass)
      ;
      .allow_subclass<IDeviceWrapper>()

*/

EMSCRIPTEN_BINDINGS(rc3200_vm) {
    function("DumbTest", &DumbTest);
    
    value_struct<vm::cpu::CpuState>("CpuState")
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
      .function("Reset",    &vm::VirtualComputer::Reset)
      .function("WriteROM", &vm::VirtualComputer::WriteROM, allow_raw_pointers())
      .function("CPUState", &vm::VirtualComputer::CPUState)
      .function("Clock",    &vm::VirtualComputer::Clock)
      .function("Step",     &vm::VirtualComputer::Step)
      .function("Tick",     &vm::VirtualComputer::Tick)
      ;

}


