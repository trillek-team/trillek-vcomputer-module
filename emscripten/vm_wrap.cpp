#include <emscripten.h>
#include <emscripten/bind.h>

#include <string>
#include <cstdio>

#include "VC.hpp"
#include "DisTR3200.hpp"
using namespace emscripten;

// Wrapper functions **********************************************************
// Generic

void SetROM_ (vm::VComputer& arr ,long ptr, size_t rom_size) {
  auto p = (vm::byte_t *)ptr;
  arr.SetROM(p, rom_size);
}

// TR3200 CPU

void SetTR3200CPU_ (vm::VComputer& arr, unsigned clock = 100000 ) {
  std::unique_ptr<vm::cpu::TR3200> cpu(new vm::cpu::TR3200(clock));
  arr.SetCPU(std::move(cpu));
}

std::string Disassembly_(vm::VComputer& arr, long pos) {
  return vm::cpu::Disassembly(arr, pos);
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
//
std::shared_ptr<vm::IDevice> GKeybtoIDevice_(std::shared_ptr<vm::dev::gkeyboard::GKeyboardDev> gk ) {
  return gk;
}

// Bindings *******************************************************************

EMSCRIPTEN_BINDINGS(trillek_vc) {

    class_<vm::VComputer>("VComputer")
      .constructor<int>()
      .function("SetTR3200CPU", &SetTR3200CPU_)
      //.function("RmCPU",      &vm::VComputer::RmCPU)
      .function("AddDevice",  &vm::VComputer::AddDevice)
      .function("GetDevice",  &vm::VComputer::GetDevice)
      .function("RmDevice",   &vm::VComputer::RmDevice)
      .function("CPUClock",   &vm::VComputer::CPUClock)
      .function("SetROM",     &SetROM_)
      .function("Reset",      &vm::VComputer::Reset)
      .function("Step",       &vm::VComputer::Step)
      .function("Tick",       &vm::VComputer::Tick)
      .function("RamSize",    &vm::VComputer::RamSize)
      //.function("Ram",        &vm::VComputer::Ram)
      .function("Disassembly",&Disassembly_)
      ;

    class_<vm::IDevice>("IDevice")
      .smart_ptr<std::shared_ptr<vm::IDevice>>()
      ;

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
      .class_function("ToIDevice", &GKeybtoIDevice_)
      ;
}


