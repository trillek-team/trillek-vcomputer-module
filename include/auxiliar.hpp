/**
 * \brief       Virtual Computer Auxiliar functions
 * \file        auxiliar.hpp
 * \copyright   The MIT License (MIT)
 *
 * Some auxiliar functions and methods
 */
#ifndef __AUXILIAR_HPP_
#define __AUXILIAR_HPP_ 1

#include "types.hpp"
#include <string>

namespace trillek {
namespace computer {

/**
 * Load a raw binary file as ROM
 * \param[in] filename Binary file with the ROM
 * \param[out] rom buffer were to write it
 * \return Read size or negative value if fails
 */
int LoadROM (const std::string& filename, Byte* rom);

} // end of namespace computer
} // end of namespace trillek

#endif // __AUXILIAR_HPP_
