#pragma once
/**
 * Trillek Virtual Computer - VSFix.hpp
 * VStudio pragmas and fix
 * Use on the .cpp files
 */

// MS Visual C++ stuff
#if defined(_MSC_VER)
  // VC++ C compiler support : C89 thanks microsoft !
  #define snprintf _snprintf
  #define log2(x) (std::log(x) / std::log(2))

  // Get bored of theses warnings
  #pragma warning(disable : 4333) // Shift warning exceeding output var size, data loss
  #pragma warning(disable : 4018) // Compare of signed and unsigned with auto conversion
  #pragma warning(disable : 4244) // Conversion of variables with data loss
  #pragma warning(disable : 4996) // _snprintf is not secure.... 

#endif

