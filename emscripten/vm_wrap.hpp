#pragma once
/**
 * Trillek Virtual Computer - vm_wrap.hpp
 * Virtual Computer wrapper for compiling with emscripten
 */

#include "VC.hpp"
#include "DisTR3200.hpp"

// Wrapper functions **********************************************************
// Generic

void SetROM_ (vm::VComputer& arr ,long ptr, size_t rom_size);

// TDA screen
void WriteTDATexture_(vm::dev::tda::TDAScreen& arr, long ptr);

std::shared_ptr<vm::IDevice> TDAtoIDevice_(std::shared_ptr<vm::dev::tda::TDADev> tda );

// Generic Keyboard
std::shared_ptr<vm::IDevice> GKeybtoIDevice_(std::shared_ptr<vm::dev::gkeyboard::GKeyboardDev> gk );

// TR3200 CPU
void SetTR3200CPU_ (vm::VComputer& arr, unsigned clock = 100000 );

void TR3200_DumpState_ (vm::VComputer& arr, vm::cpu::TR3200State& state) {
  arr.GetState(&state, sizeof state);
}

vm::dword_t TR3200_GetReg_ (vm::cpu::TR3200State& arr, unsigned x) {
  return arr.r[x];
}

vm::dword_t TR3200_GetPC_ (vm::cpu::TR3200State& arr) {
  return arr.pc;
}


