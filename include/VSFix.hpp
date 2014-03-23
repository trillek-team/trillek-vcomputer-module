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

  // Get bored of theses warnings
  #pragma warning(disable : 4333) // Shift warning execding output var size, data loss
  #pragma warning(disable : 4018) // Comparation of signed and unsigned with auto conversion
  #pragma warning(disable : 4244) // Conversion of variables with data loss

#endif

