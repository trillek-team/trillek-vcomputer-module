/**
 *
 */
#include "glerror.hpp"
#include "config_main.hpp"

#include <iostream>
#include <string>

namespace trillek {

void tr_check_gl_error(const char *file, int line) {
#ifndef NDEBUG
    GLenum err (glGetError());
    unsigned count =0;
    while(err!=GL_NO_ERROR) {
        std::string error;

        switch(err) {
        case GL_INVALID_OPERATION:
            error="INVALID_OPERATION";
            break;
        case GL_INVALID_ENUM:
            error="INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error="INVALID_VALUE";
            break;
        case GL_OUT_OF_MEMORY:
            error="OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error="INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            error="UNKNOW_ERROR";
        }
        std::cerr << "#" << count << " " << err << ": GL_" << error.c_str()
                  << " - " << file << ":" << line << std::endl;
        err=glGetError();
        count++;
    }
#endif
}
} // End of namespace trillek

