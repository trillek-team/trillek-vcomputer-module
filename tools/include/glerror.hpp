/**
 * Trillek Virtual Computer - glerror.hpp
 * Function/macros to debug OpenGL code
 */

#ifndef __TR_GL_ERROR_HPP_
#define __TR_GL_ERROR_HPP_ 1

namespace trillek {
    /**
     * Check if there is a OpenGL error and display it
     * @param file Source file
     * @param line Source line
     */
    void tr_check_gl_error(const char *file, int line);
} // End of namespace trillek

#ifndef NDEBUG
/**
 * Usage
 * [... some opengl calls]
 * glCheckError();
 */
#define check_gl_error() trillek::tr_check_gl_error(__FILE__,__LINE__)

#else

#define check_gl_error()

#endif

#endif // __TR_GL_ERROR_HPP_

