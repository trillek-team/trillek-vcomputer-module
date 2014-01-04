#pragma once
#ifndef __AUX_HPP__
#define __AUX_HPP__ 1
/**
 * Some semi-generic auxiliar functions related to the vm
 */

#include <string>

#include "VComputer.hpp"

namespace vm {
namespace aux {

  /**
   * Load a raw binary file as ROM
   * \param filename Binary file with the ROM, max size 64KiB
   * \param vcomp Virtual Computer
   * \return Read size or negative value if fails
   */
  unsigned LoadROM (const std::string& filename, vm::VirtualComputer& vcomp);

  /**
   * Maps GLFW3 Key codes to TR3200 keycodes
   */
  byte_t GLFWKeyToTR3200 (int key);

  /**
   * Maps SDL2 Scancodes to TR3200 keycodes
   */
  byte_t SDL2KeyToTR3200 (int key);

} // end of namespace aux
} // end of namespace vm

#endif // __AUX_HPP__

