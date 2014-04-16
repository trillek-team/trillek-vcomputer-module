#include <emscripten.h>
#include <emscripten/bind.h>

#include <string>
#include <cstdio>

#include "vm_wrap.hpp"

using namespace emscripten;
using namespace vm;

// Bindings *******************************************************************

EMSCRIPTEN_BINDINGS(trillek_vc) {
    // General stuff
    class_<vm::VComputer>("VComputer")
      .constructor<int>()
      //.function("RmCPU",        &vm::VComputer::RmCPU)
      .function("AddDevice",      &vm::VComputer::AddDevice)
      .function("GetDevice",      &vm::VComputer::GetDevice)
      .function("RmDevice",       &vm::VComputer::RmDevice)
      .function("CPUClock",       &vm::VComputer::CPUClock)
      .function("SetROM",         &SetROM_)
      .function("Reset",          &vm::VComputer::Reset)
      .function("Step",           &vm::VComputer::Step)
      .function("Tick",           &vm::VComputer::Tick)
      .function("RamSize",        &vm::VComputer::RamSize)
      .function("Ram",            &vm::VComputer::Ram, allow_raw_pointers())
      ;

    class_<vm::IDevice>("IDevice")
      .smart_ptr<std::shared_ptr<vm::IDevice>>()
      // TODO Add stuff to allow to create a derived class of this in JS
      ;

    // TR3200 stuff
    function("SetTR3200CPU",        &SetTR3200CPU_);
    function("DisassemblyTR3200",   &vm::cpu::DisassemblyTR3200);

    class_<vm::cpu::TR3200State>("TR3200State")
      .constructor()
      .function("GetR",             &TR3200_GetReg_)
      .function("GetPC",            &TR3200_GetPC_)
      /*
      .field("wait_cycles",     &vm::cpu::TR3200State::wait_cycles)
      .field("int_msg",         &vm::cpu::TR3200State::int_msg)
      .field("interrupt",       &vm::cpu::TR3200State::interrupt)
      //.field("step_mode"        &vm::cpu::TR3200State::step_mode) // WTF!
      .field("skiping",         &vm::cpu::TR3200State::skiping)
      .field("sleeping",        &vm::cpu::TR3200State::sleeping)
      */
      ;

    function("GetTR3200State",      &TR3200_DumpState_);


    // TDA stuff
    class_<vm::dev::tda::TDAScreen>("TDAScreen")
      .constructor()
      .function("toRGBATexture",&WriteTDATexture_)
      ;

    class_<vm::dev::tda::TDADev>("TDADev")
      .smart_ptr_constructor(   &std::make_shared<vm::dev::tda::TDADev>)
      .function("DumpScreen",   &vm::dev::tda::TDADev::DumpScreen)
      .function("DoVSync",      &vm::dev::tda::TDADev::DoVSync)
      .class_function("ToIDevice", &TDAtoIDevice_)
      ;

    // Genetic Keyboard
    class_<vm::dev::gkeyboard::GKeyboardDev>("GKeyboardDev")
      .smart_ptr_constructor(   &std::make_shared<vm::dev::gkeyboard::GKeyboardDev>)
      .function("SendKeyEvent", &vm::dev::gkeyboard::GKeyboardDev::SendKeyEvent)
      .function("EnforceSendKeyEvent", &vm::dev::gkeyboard::GKeyboardDev::EnforceSendKeyEvent)
      .class_function("ToIDevice", &GKeybtoIDevice_)
      ;
}



// Wrapper functions **********************************************************
// Generic

void SetROM_ (vm::VComputer& arr ,long ptr, size_t rom_size) {
  auto p = (vm::byte_t *)ptr;
  arr.SetROM(p, rom_size);
}

// TR3200 CPU

void SetTR3200CPU_ (vm::VComputer& arr, unsigned clock) {
  std::unique_ptr<vm::cpu::TR3200> cpu(new vm::cpu::TR3200(clock));
  arr.SetCPU(std::move(cpu));
}

// TDA screen

void WriteTDATexture_(vm::dev::tda::TDAScreen& arr, long ptr) {
  auto p = (vm::dword_t *)ptr;
  vm::dev::tda::TDAtoRGBATexture(arr, p);
}

std::shared_ptr<vm::IDevice> TDAtoIDevice_(std::shared_ptr<vm::dev::tda::TDADev> tda ) {
  return tda;
}

// Generic Keyboard
std::shared_ptr<vm::IDevice> GKeybtoIDevice_(std::shared_ptr<vm::dev::gkeyboard::GKeyboardDev> gk ) {
  return gk;
}

