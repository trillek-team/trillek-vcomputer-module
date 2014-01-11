#include <emscripten.h>
#include <emscripten/bind.h>

#include <string>
#include <cstdio>

#include "VM.hpp"
using namespace emscripten;

// Wrapper functions

void LoadROM_ (const std::string& filename, vm::VirtualComputer<vm::cpu::TR3200>& vcomp) {
	vm::aux::LoadROM(filename, vcomp);
}

void WriteROM_ (vm::VirtualComputer<vm::cpu::TR3200>& arr,long ptr, size_t rom_size) {
  auto p = (vm::byte_t *)ptr;
  arr.WriteROM(p, rom_size);
}

bool AddGKey_(vm::VirtualComputer<vm::cpu::TR3200>& arr, unsigned slot, vm::keyboard::GKeyboard& d ) {
  return arr.AddDevice(slot, d);
}

bool AddCDA_(vm::VirtualComputer<vm::cpu::TR3200>& arr, unsigned slot, vm::cda::CDA& d ) {
  return arr.AddDevice(slot, d);
}

void WriteTexture_(vm::cda::CDA& arr, long ptr) {
  auto p = (vm::byte_t *)ptr;
  arr.ToRGBATexture((vm::dword_t*)p); // TODO Perhaps we need a special function for this case
}

std::string Disassembly_(vm::VirtualComputer<vm::cpu::TR3200>& arr) {
  return vm::cpu::Disassembly(arr.RAM(), arr.CPU().PC());
}

vm::dword_t Reg_(vm::VirtualComputer<vm::cpu::TR3200>& arr, unsigned r) {
	return arr.CPU().Reg(r);
}

vm::dword_t PC_(vm::VirtualComputer<vm::cpu::TR3200>& arr) {
	return arr.CPU().PC();
}

vm::dword_t Sleeping_(vm::VirtualComputer<vm::cpu::TR3200>& arr) {
	return arr.CPU().Sleeping();
}

vm::dword_t Skiping_(vm::VirtualComputer<vm::cpu::TR3200>& arr) {
	return arr.CPU().Skiping();
}

EMSCRIPTEN_BINDINGS(tr3200_vm) {
    function("LoadROM",     &LoadROM_);

    class_<vm::VirtualComputer<vm::cpu::TR3200>>("VirtualComputer")
      .constructor<int>()
      .function("Reset",      &vm::VirtualComputer<vm::cpu::TR3200>::Reset)
      .function("WriteROM",   &WriteROM_)
      .function("AddKeyboard",&AddGKey_)
      .function("AddCDA",     &AddCDA_)
      .function("RemoveDevice",  &vm::VirtualComputer<vm::cpu::TR3200>::RemoveDevice)
      .function("Clock",      &vm::VirtualComputer<vm::cpu::TR3200>::Clock)
      .function("Step",       &vm::VirtualComputer<vm::cpu::TR3200>::Step)
      .function("Tick",       &vm::VirtualComputer<vm::cpu::TR3200>::Tick)
      .function("Reg",        &Reg_)
      .function("PC",         &PC_)
      .function("Skiping",    &Skiping_)
      .function("Sleeping",   &Sleeping_)
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


