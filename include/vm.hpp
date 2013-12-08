#pragma once
#ifndef ___VM_HPP__
#define ___VM_HPP__ 1
/**
 * RC3200 VM - vm.hpp
 * Main header file that includes all
 */

// MS Visual C++ stuff
#if defined(_MSC_VER)
        // VC++ C compiler support : C89 thanks microsoft !
        #define snprintf _snprintf 
        
        // Get bored of theses warnings
        #pragma warning(disable : 4996) // Ni puñetera idea
        #pragma warning(disable : 4333) // Shift warning execding output var size, data loss
        #pragma warning(disable : 4018) // Comparation of signed and unsigned with auto conversion
        #pragma warning(disable : 4244) // Conversion of variables with data loss
#endif

#include "types.hpp"

#include "ram.hpp"
#include "cpu.hpp"

#include "IDevice.hpp"
#include "vcomputer.hpp"

// Devices
#include "CDA.hpp"
#include "GKeyboard.hpp"

// Misc
#include "dis_rc3200.hpp"


#endif // ___VM_HPP__
