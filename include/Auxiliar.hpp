#pragma once
/*!
 * \brief       Virtual Computer Auxiliar functions
 * \file        Auxiliar.hpp
 * \copyright   The MIT License (MIT)
 *
 * Some auxiliar functions and methods
 */

#include "Types.hpp"
#include <string>

namespace vm {
    namespace aux {

        /*!
         * Load a raw binary file as ROM
         * \param[in] filename Binary file with the ROM
         * \param[out] rom buffer were to write it
         * \return Read size or negative value if fails
         */
        int LoadROM (const std::string& filename, byte_t* rom);

    } // end of namespace aux
} // end of namespace vm

