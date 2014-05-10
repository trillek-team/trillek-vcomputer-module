/**
 * \brief       Virtual Computer Auxiliar functions
 * \file        suxiliar.hpp
 * \copyright   The MIT License (MIT)
 *
 * Some auxiliar functions and methods
 */
#ifndef __AUXILIAR_HPP_
#define __AUXILIAR_HPP_ 1

#include "types.hpp"
#include <string>

namespace vm {
namespace aux {

/**
 * Load a raw binary file as ROM
 * \param[in] filename Binary file with the ROM
 * \param[out] rom buffer were to write it
 * \return Read size or negative value if fails
 */
int LoadROM (const std::string& filename, Byte* rom);

} // end of namespace aux
} // end of namespace vm

#endif // __AUXILIAR_HPP_
