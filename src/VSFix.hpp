/**
 * \brief       Fix or hides VS2012 warings and errors
 * \file        VSFix.hpp
 * \copyright   The MIT License (MIT)
 *
 * VStudio pragmas and fix
 * Use ONLY on the .cpp files
 */
#ifndef __VSFIX_HPP_
#define __VSFIX_HPP_ 1

// MS Visual C++ stuff
#if defined(_MSC_VER)
// VC++ C compiler support : C89 thanks microsoft !
    #define snprintf _snprintf
    #define log2(x) ( std::log(x) / std::log(2) )

// Get bored of theses warnings
    #pragma warning(disable : 4333) // Shift warning exceeding output var size,
                                    // data loss
    #pragma warning(disable : 4018) // Compare of signed and unsigned with auto
                                    // conversion
    #pragma warning(disable : 4244) // Conversion of variables with data loss
    #pragma warning(disable : 4996) // _snprintf is not secure....

#endif // if defined(_MSC_VER)

#endif // __VSFIX_HPP_
