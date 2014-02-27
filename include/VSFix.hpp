#pragma once
/**
 * Trillek Virtual Computer - VSFix.hpp
 * VStudio pragmas and fix
 */

#ifndef ___VSFIX_HPP__
#define ___VSFIX_HPP__ 1

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


#endif // ___VSFIX_HPP__

